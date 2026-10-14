#include "wrap.h"

#include "plot.h"
#include "text.h"
#include "walk.h"

#include <algorithm>

namespace text {
namespace {

// The display color state an atom establishes; 0 = none.
u16 atom_carry_color(const Atom& atom) {
    if (atom.kind == AtomKind::PlayerColor && atom.payload) {
        return u16(atom.payload << 8) | std::to_underlying(TextColor::PlayerColor);
    }
    if (atom.kind == AtomKind::Control && is_color_code(atom.control)) {
        return atom.control;
    }
    return 0;
}

LineBreak line_ended(const WalkSummary& summary, s32 x_start, u16 carry_color) {
    return {summary.cursor, summary.x - x_start, summary.stop, carry_color};
}

}

LineBreak find_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap) {
    const u8* cursor = start;
    if (word_wrap) {
        LineBreak stopped{};
        const u8* word_start = start;
        s32 word_start_x = x_start;
        s32 word_end_x = x_start;
        bool in_word = false;
        u16 carry_color = 0;
        const auto sink = [&](const Atom& atom) {
            if (atom.kind == AtomKind::Glyph || atom.kind == AtomKind::Icon) {
                if (!in_word) {
                    in_word = true;
                    word_start = atom.start;
                    word_start_x = atom.x - atom.advance;
                }
                word_end_x = atom.x;
                return true;
            }
            if (atom.kind == AtomKind::Space || atom.kind == AtomKind::Tab) {
                // the word just ended. If the word overflowed, it moves whole
                if (in_word && word_end_x > clip_right) {
                    stopped = {word_start, word_start_x - x_start, WalkStop::Sink, carry_color};
                    return false;
                }
                in_word = false;
                word_start = atom.next;
                word_start_x = atom.x;
            }
            if (const u16 atom_color = atom_carry_color(atom)) {
                carry_color = atom_color;
            }
            return true;
        };
        const WalkSummary summary = walk_line({font, start, x_start, x_origin}, cursor, end, sink);
        if (stopped.break_at) {
            return stopped;
        }

        if (in_word && word_end_x > clip_right) {
            return {word_start, word_start_x - x_start, WalkStop::Sink, carry_color};
        }
        return line_ended(summary, x_start, carry_color);
    }

    LineBreak stopped{};
    u16 carry_color = 0;
    const auto sink = [&](const Atom& atom) {
        if ((atom.kind == AtomKind::Glyph || atom.kind == AtomKind::Icon) && atom.x > clip_right) {
            stopped = {atom.start, atom.x - atom.advance - x_start, WalkStop::Sink, carry_color};
            return false;
        }
        if (const u16 atom_color = atom_carry_color(atom)) {
            carry_color = atom_color;
        }
        return true;
    };
    const WalkSummary summary = walk_line({font, start, x_start, x_origin}, cursor, end, sink);
    if (stopped.break_at) {
        return stopped;
    }
    return line_ended(summary, x_start, carry_color);
}

LineBreak find_line_break(u8 slot, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap) {
    return find_line_break(TextSystem::instance().font_for_slot(slot), start, end, x_start, x_origin, clip_right, word_wrap);
}

size_t fit_text_to_width(u8 slot, std::string_view text, s32 max_width) {
    const PrintFont font = TextSystem::instance().font_for_slot(slot);
    const u8* start = (const u8*)text.data();
    const u8* end = start + text.size();
    const u8* cursor = start;
    while (cursor < end) {
        const u8* stop_at = nullptr;
        const auto sink = [&](const Atom& atom) {
            if (atom.x > max_width) {
                stop_at = atom.start;
                return false;
            }
            return true;
        };
        const WalkSummary summary = walk_line({font, start, 0, 0}, cursor, end, sink);
        if (stop_at) {
            return size_t(stop_at - start);
        }
        if (summary.stop == WalkStop::Newline) {
            cursor = summary.cursor; // width resets at the line break
            continue;
        }
        if (summary.stop == WalkStop::PrintEnd) {
            return size_t(summary.stop_byte - start);
        }
        break; // End: everything fits
    }
    return text.size();
}

s32 max_line_width(u8 slot, std::string_view text) {
    const PrintFont font = TextSystem::instance().font_for_slot(slot);
    const u8* start = (const u8*)text.data();
    const u8* end = start + text.size();
    const u8* cursor = start;
    s32 widest = 0;
    const auto sink = [](const Atom&) {
        return true;
    };
    while (cursor < end) {
        const WalkSummary summary = walk_line({font, start, 0, 0}, cursor, end, sink);
        widest = std::max(widest, summary.x);
        if (summary.stop != WalkStop::Newline) {
            break;
        }
        cursor = summary.cursor;
    }
    return widest;
}

s32 cursor_width(const PrintFont& font) {
    return font.cursor_advance ? font.cursor_advance + font.char_spacing : 0;
}

} // namespace text
