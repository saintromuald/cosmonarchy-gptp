#include "screen_text.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../BW/enumerations.h"
#include "../interface/mainmenu/glucustom.h"
#include "../interface/uifeatures/ui_features.h"
#include "../../systems/display/zoom.h"

namespace hooks {
namespace {

// Whether the game layer asked for the text this frame. The text pass is not
// idempotent bookkeeping - it is one draw per frame - so the deferred half
// draws only when the deferring half ran.
bool g_screen_text_pending = false;

char* slot_text(u32 slot) {
    return &g_text_slot_buffers[slot * TextSlotSize];
}

void draw_slot_text(u32 slot, s32 left, s32 top, s32 right, u8 alignment, u16 color) {
    helper::get_color_handle(color);
    g_draw_text.left = left;
    g_draw_text.top = top;
    g_draw_text.right = right;
    g_draw_text.bottom = top + g_text_area_y;
    g_print_xy_alignment = alignment;
    helper::bw_print_xy(slot_text(slot), left, top);
}

// 0048CF60, reimplemented. Draws the message log ring oldest line first, then
// the center notification line, then the top-right line, then the UI features that
// hooked vanilla's exit call. Ring colors come from display_text_color, the pcolor
// array the message printer writes per slot.
void draw_engine_screen_text() {
    helper::set_font_handle((u32*)font_base[1]); // In-game font table, as vanilla selects

    u32 slot = g_text_line_first;
    s32 y = MessageLogTop;
    for (u32 line = 0; line < MessageLogSlotCount; ++line) {
        if (slot_text(slot)[0] != '\0') {
            draw_slot_text(slot, MessageLogLeft, y, MessageLogRight, MessageLogPrependByte, display_text_color[slot]);
        }
        y += g_text_line_height;
        slot = (slot + 1) % MessageLogSlotCount;
    }

    if (slot_text(CenterTextSlot)[0] != '\0') {
        draw_slot_text(CenterTextSlot, CenterTextLeft, CenterTextTop, CenterTextRight,
            CenterTextAlignment, text_bytes[CenterTextSlot]);
    }

    if (slot_text(TopRightTextSlot)[0] != '\0') {
        draw_slot_text(TopRightTextSlot, TopRightTextLeft, TopRightTextTop, TopRightTextRight,
            TopRightTextAlignment, text_bytes[TopRightTextSlot]);
    }

    draw_ui_features(); // The old exit hook (0048D0A9) drew these after the text pass
    helper::set_font_handle(nullptr);
}

} // namespace

// 0048CE90, reimplemented. Vanilla pads the per-line step so the log bottom lands on the
// center text band; deriving the span from the layout constants keeps that true after
// either band moves.
void __cdecl init_screen_text_layout() {
    helper::set_font_handle((u32*)font_base[1]);

    const s32 band_height = g_line_spacing + g_current_font_height;
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