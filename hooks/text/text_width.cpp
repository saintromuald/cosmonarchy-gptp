#include "text_width.h"

#include "BW/bwdata.h"
#include "BW/structures.h"
#include "hooks/interface/mainmenu/glucustom.h"
#include "systems/text/measure.h"
#include "systems/text/text.h"
#include "systems/text/walk.h"
#include "systems/text/wrap.h"

#include <cstring>

namespace hooks {
namespace {

text::PrintFont current_print_font() {
    const u8 slot = text::slot_of(g_current_font).value_or(1);
    return text::TextSystem::instance().font_for_slot(slot);
}

// One entry per codepoint of a field's text: byte offset + advance.
struct GlyphSpan {
    u32 offset;
    s32 width;
};

s32 collect_glyph_spans(const text::PrintFont& font, const u8* text, GlyphSpan* spans, s32 capacity) {
    s32 count = 0;
    const auto sink = [&](const text::Atom& atom) {
        if (count < capacity) {
            spans[count] = {u32(atom.start - text), atom.advance};
            ++count;
        }
        return true;
    };
    const u8* start = text;
    text::walk_line({font, start, 0, 0}, start, start + strlen((const char*)start), sink);
    return count;
}

}

const u8* __stdcall font_find_line_break_impl(const u8* text) {
    static u16 carry = 0;
    static const u8* previous_break = nullptr;
    if (text != previous_break) {
        carry = 0;
    }
    g_split_slot_color = carry;
    const text::PrintFont font = current_print_font();
    const s32 start = g_current_text_x;
    const text::LineBreak line = text::find_line_break(font, text, text + strlen((const char*)text),
        start, g_draw_text.left, g_draw_text.right, g_word_wrap_mode != 0);
    g_current_text_x = start + line.width;
    if (line.break_at != text) {
        previous_break = line.break_at;
        carry = line.carry_color ? line.carry_color : carry;
    }
    return line.break_at;
}

u32 __stdcall font_fit_text_to_width_impl(const u8* text, s32 max_width) {
    const u8 slot = text::slot_of(g_current_font).value_or(1);
    return text::fit_text_to_width(slot, {(const char*)text, strlen((const char*)text)}, max_width);
}

s32 __stdcall font_max_line_width_impl(const u8* text) {
    const u8 slot = text::slot_of(g_current_font).value_or(1);
    return text::max_line_width(slot, {(const char*)text, strlen((const char*)text)});
}

void __stdcall textbox_scroll_to_cursor_impl(BinDlg* dlg) {
    bw::TextInputData& data = *reinterpret_cast<bw::TextInputData*>(&dlg->TextBox0x36);
    const u8* text = (const u8*)dlg->text;
    const Box16& bounds = dlg->bounds;
    const text::PrintFont font = current_print_font();
    const s32 cursor_width = text::cursor_width(font);

    const s32 available = bounds.right - data.horizontal_margin * 2 - bounds.left - cursor_width + 1;
    data.visible_width = u16(available);

    // u8 scroll offsets cap reachable text at 255 bytes, so 256 spans cover it.
    constexpr s32 MaxGlyphSpans = 256;
    GlyphSpan spans[MaxGlyphSpans];
    const s32 span_count = collect_glyph_spans(font, text, spans, MaxGlyphSpans);

    if (g_text_input_mode == 1) {
        if (data.cursor_position < data.scroll_offset) {
            data.scroll_offset = data.cursor_position;
            return;
        }
        s32 accum = 0;
        for (s32 i = 0; i < span_count && spans[i].offset < data.cursor_position; ++i) {
            if (spans[i].offset >= data.scroll_offset) {
                accum += spans[i].width;
            }
        }
        if (accum < available) {
            return; // the cursor is inside the visible view
        }
        // Snap the view to the cursor, then show as much text before it as fits.
        data.scroll_offset = data.cursor_position;
        s32 remaining = available;
        for (s32 i = span_count - 1; i >= 0; --i) {
            if (spans[i].offset >= data.cursor_position) {
                continue;
            }
            if (spans[i].width >= remaining) {
                return;
            }
            remaining -= spans[i].width;
            data.scroll_offset = u8(spans[i].offset);
            if (data.scroll_offset == 0) {
                return;
            }
        }
        return;
    }

    if (g_text_input_mode == 2) {
        // Follow the tail: the largest suffix that fits, then keep the cursor visible.
        data.scroll_offset = u8(strlen((const char*)text));
        s32 remaining = available;
        for (s32 i = span_count - 1; i >= 0; --i) {
            if (spans[i].width >= remaining) {
                break;
            }
            remaining -= spans[i].width;
            data.scroll_offset = u8(spans[i].offset);
        }
        if (data.scroll_offset > data.cursor_position) {
            data.scroll_offset = data.cursor_position;
        }
    }
}

} // hooks
