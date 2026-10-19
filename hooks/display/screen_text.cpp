#include "screen_text.h"
#include "grid.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../BW/enumerations.h"
#include "logger.h"
#include "player_color.h"
#include "../interface/mainmenu/glucustom.h"
#include "../interface/uifeatures/ui_features.h"
#include "../../systems/display/zoom.h"
#include "../../systems/text/plot.h"
#include "../../systems/text/text.h"
#include "../../systems/text/wrap.h"

#include <algorithm>
#include <format>
#include <string_view>

namespace hooks {
namespace {

constexpr s32 SlotTextCap = s32(TextSlotSize) - 1;

// Whether the game layer asked for the text this frame. The text pass is not
// idempotent bookkeeping - it is one draw per frame - so the deferred half
// draws only when the deferring half ran.
bool g_screen_text_pending = false;

// Per-slot ring message colors: written by display_text_message (0x0048CD30's replacement), read by the draw below.
text::ColorState g_display_text_color[MessageLogSlotCount];

char* slot_text(u32 slot) {
    return &g_text_slot_buffers[slot * TextSlotSize];
}

u16 message_color(u16 packed) {
    const u8 code = u8(packed);
    const u8 payload = u8(packed >> 8);
    const bool pair_code = code == std::to_underlying(TextColor::PlayerColor) || code == std::to_underlying(TextColor::PlayerColor2);
    // Packed pair colors select the row directly, embedded pairs use the sparse payload map (player_font_ramp_row).
    // The high byte is garbage in the non-pair paths, so it is ignored there.
    if (pair_code && payload != 0 && payload <= PlayerColorRows) {
        return text::pair_state(u8(payload - 1));
    }
    if (text::is_color_code(code)) {
        return code;
    }
    static bool logged = false;
    if (!logged) {
        logged = true;
        gptp::g_logger << std::format("display_text_message: {:#x} is not a message color, using grey\n", packed);
    }
    return std::to_underlying(TextColor::Grey);
}

void copy_slot_text(u32 slot, const text::PrintFont& font, std::string_view line) {
    char* destination = slot_text(slot);
    if (line.size() > size_t(SlotTextCap)) {
        // The cap snaps to an atom boundary so it cannot split a utf-8 sequence or a player pair.
        // Bytes up to the wrap point are still consumed.
        const u8* start = (const u8*)line.data();
        const auto sink = [&](const text::Atom& atom) {
            return atom.next - start <= SlotTextCap;
        };
        const text::WalkSummary summary = text::walk_line({font, start, 0, 0}, start, start + line.size(), sink);
        const size_t capped = summary.stop == text::WalkStop::Sink
            ? size_t(summary.stop_byte - start)
            : size_t(SlotTextCap);
        line = line.substr(0, capped);
    }
    std::copy_n(line.begin(), line.size(), destination);
    destination[line.size()] = '\0';
}

void apply_slot_color(text::ColorState state) {
    const auto resolve = [&](u16 value, u8 pair_slot) -> const u8* {
        if (text::is_pair_state(value)) {
            set_player_font_ramp(text::pair_state_row(value), pair_slot);
            return &g_tfontgame_image_data[pair_slot * 8];
        }
        const u8 code = text::is_color_code(u8(value)) ? u8(value) : std::to_underlying(TextColor::Grey);
        return text::ramp(text::color_slot(code));
    };
    g_current_font_color = u32(resolve(state.current, PlayerFontSlot));
    g_saved_font_color = u32(resolve(state.saved, u8(PlayerFontSlot + 1)));
}

void draw_slot_text(u32 slot, s32 left, s32 top, s32 right, u8 alignment) {
    g_draw_text.left = left;
    g_draw_text.top = top;
    g_draw_text.right = right;
    g_draw_text.bottom = top + g_text_area_y;
    g_print_xy_alignment = alignment;
    helper::bw_print_xy(slot_text(slot), left, top);
}

// 0048CF60, reimplemented. Draws the message log ring oldest line first, then
// the center notification line, then the top-right line, then the UI features that
// hooked vanilla's exit call. Ring colors come from g_display_text_color, the
// state array the message writer stores per slot.
void draw_engine_screen_text() {
    helper::set_font_handle((u32*)font_base[1]); // In-game font table, as vanilla selects

    u32 slot = g_text_line_first;
    s32 y = MessageLogTop;
    for (u32 line = 0; line < MessageLogSlotCount; ++line) {
        if (slot_text(slot)[0] != '\0') {
            apply_slot_color(g_display_text_color[slot]);
            draw_slot_text(slot, MessageLogLeft, y, MessageLogRight, MessageLogPrependByte);
        }
        y += g_text_line_height;
        slot = (slot + 1) % MessageLogSlotCount;
    }

    if (slot_text(CenterTextSlot)[0] != '\0') {
        helper::get_color_handle(text_bytes[CenterTextSlot]);
        draw_slot_text(CenterTextSlot, CenterTextLeft, CenterTextTop, CenterTextRight, CenterTextAlignment);
    }

    if (slot_text(TopRightTextSlot)[0] != '\0') {
        helper::get_color_handle(text_bytes[TopRightTextSlot]);
        draw_slot_text(TopRightTextSlot, TopRightTextLeft, TopRightTextTop, TopRightTextRight, TopRightTextAlignment);
    }

    draw_ui_features(); // The old exit hook (0048D0A9) drew these after the text pass
    helper::set_font_handle(nullptr);
}

} // namespace

void display_text_message(const char* text, u16 packed_color, u32 ticks, u8 from_trigger) {
    if (!text) {
        return;
    }
    const text::PrintFont font = text::TextSystem::instance().font_for_slot(1);
    const u16 base = message_color(packed_color);
    text::ColorState carry{base, base};
    std::string_view remaining{text};
    while (!remaining.empty()) {
        const text::LineBreak line = text::next_line_break(font, remaining,
            MessageLogLeft, MessageLogLeft, MessageLogRight, true, carry);
        const size_t consumed = size_t(line.break_at - (const u8*)remaining.data());
        const u32 slot = g_text_line_first;
        g_text_line_first = u8((slot + 1) % MessageLogSlotCount);
        g_display_text_color[slot] = carry;
        copy_slot_text(slot, font, remaining.substr(0, consumed));
        g_text_slot_timers[slot] = ticks;
        g_text_slot_from_trigger[slot] = from_trigger;
        remaining.remove_prefix(consumed);
        carry = (line.stop == text::WalkStop::Newline || line.stop == text::WalkStop::PrintEnd)
            ? text::ColorState{base, line.carry.current}
            : line.carry;
    }
    for (u32 slot = 0; slot < MessageLogSlotCount; ++slot) {
        if (slot_text(slot)[0] != '\0') {
            expire_text_line(slot);
        }
    }
    // Vanilla leaves the insert clip and a cleared font handle behind.
    g_draw_text.left = 0;
    g_draw_text.right = 620;
    helper::set_font_handle(nullptr);
}

// 0048CE90, reimplemented. Vanilla pads the per-line step so the log bottom lands on
// the center text band; deriving the span from the layout constants keeps that true after
// either band moves.
void __cdecl init_screen_text_layout() {
    helper::set_font_handle((u32*)font_base[1]);

    const s32 band_height = text::TextSystem::instance().line_advance(1);
    g_text_area_y = band_height;
    g_text_line_height = band_height;

    const s32 shortfall = (CenterTextTop - MessageLogTop) - MessageLogSlotCount * band_height;
    if (shortfall > 0) {
        g_text_line_height += shortfall / MessageLogSlotCount;
    }

    helper::set_font_handle(nullptr);
    g_text_line_first = 10; // The first message lands on the top line
}

void draw_screen_text() {
    if (!g_zoom.magnified()) {
        draw_engine_screen_text();
        return;
    }

    g_screen_text_pending = true;
}

void draw_deferred_screen_text() {
    if (!g_screen_text_pending) {
        return;
    }

    g_screen_text_pending = false;
    draw_engine_screen_text();
}

} // hooks