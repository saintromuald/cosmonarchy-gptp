#include "wrap.h"

#include "plot.h"
#include "player_color.h"
#include "text.h"
#include "walk.h"

#include <algorithm>

namespace text {
namespace {

constexpr u16 GreyOverride = std::to_underlying(TextColor::DarkGrey);

ColorState apply_color_atom(ColorState state, const Atom& atom) {
    switch (color_step(state.current == GreyOverride, atom)) {
        case ColorStep::Push:
            state.saved = state.current;
            state.current = atom.kind == AtomKind::PlayerColor
                ? pair_state(player_font_ramp_row(atom.payload))
                : atom.control;
            break;
        case ColorStep::Revert:
            state.current = state.saved;
            break;
        case ColorStep::None:
            break;
    }
    return state;
}

LineBreak line_ended(const WalkSummary& summary, s32 x_start, ColorState carry, bool rendered) {
    return {summary.cursor, summary.x - x_start, summary.stop, rendered, carry};
}

}

u16 pair_state(u8 row) {
    return u16((u16(row + 1) << 8) | std::to_underlying(TextColor::PlayerColor));
}

u8 pair_state_row(u16 state) {
    return u8(state >> 8) - 1;
}

bool is_pair_state(u16 state) {
    return u8(state) == std::to_underlying(TextColor::PlayerColor) && u8(state >> 8) != 0;
}

// The grey override swallows the legacy codes and player pairs.
// The extended colors bypass it (writeFontApplyColour's off_4 jump).
ColorStep color_step(bool grey_current, const Atom& atom) {
    switch (atom.kind) {
        case AtomKind::PlayerColor:
            return grey_current || atom.payload == 0 ? ColorStep::None : ColorStep::Push;
        case AtomKind::Control:
            if (grey_current && atom.control < 0x20) {
                return ColorStep::None;
            }
            if (atom.control == std::to_underlying(TextColor::Revert)) {
                return ColorStep::Revert;
            }
            return is_color_code(atom.control) ? ColorStep::Push : ColorStep::None;
        default:
            return ColorStep::None;
    }
}

LineBreak find_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state) {
    const u8* cursor = start;
    if (word_wrap) {
        LineBreak stopped{};
        const u8* word_start = start;
        s32 word_start_x = x_start;
        s32 word_end_x = x_start;
        bool in_word = false;
        bool rendered = false;
        bool word_rendered = false; // rendered as of the moved word's first glyph
        ColorState carry = start_state;
        ColorState word_state = start_state; // the state at the moved word's first glyph
        const auto sink = [&](const Atom& atom) {
            if (atom.kind == AtomKind::Glyph || atom.kind == AtomKind::Icon) {
                if (!in_word) {
                    in_word = true;
                    word_start = atom.start;
                    word_start_x = atom.x - atom.advance;
                    word_state = carry;
                    word_rendered = rendered;
                }
                word_end_x = atom.x;
                rendered = true;
                return true;
            }
            if (atom.kind == AtomKind::Space || atom.kind == AtomKind::Tab) {
                // the word just ended. If the word overflowed, it moves whole
                if (in_word && word_end_x > clip_right) {
                    stopped = {word_start, word_start_x - x_start, WalkStop::Sink, word_rendered, word_state, word_end_x - word_start_x};
                    return false;
                }
                in_word = false;
                word_start = atom.next;
                word_start_x = atom.x;
            }
            carry = apply_color_atom(carry, atom);
            return true;
        };
        const WalkSummary summary = walk_line({font, start, x_start, x_origin}, cursor, end, sink);
        if (stopped.break_at) {
            return stopped;
        }

        if (in_word && word_end_x > clip_right) {
            return {word_start, word_start_x - x_start, WalkStop::Sink, word_rendered, word_state, word_end_x - word_start_x};
        }
        return line_ended(summary, x_start, carry, rendered);
    }

    LineBreak stopped{};
    bool rendered = false;
    ColorState carry = start_state;
    const auto sink = [&](const Atom& atom) {
        if (atom.kind == AtomKind::Glyph || atom.kind == AtomKind::Icon) {
            if (atom.x > clip_right) {
                stopped = {atom.start, atom.x - atom.advance - x_start, WalkStop::Sink, rendered, carry};
                return false;
            }
            rendered = true;
            return true;
        }
        carry = apply_color_atom(carry, atom);
        return true;
    };
    const WalkSummary summary = walk_line({font, start, x_start, x_origin}, cursor, end, sink);
    if (stopped.break_at) {
        return stopped;
    }
    return line_ended(summary, x_start, carry, rendered);
}

LineBreak next_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state) {
    LineBreak line = find_line_break(font, start, end, x_start, x_origin, clip_right, word_wrap, start_state);
    if (word_wrap && line.stop == WalkStop::Sink
        && (line.break_at == start
            || (!line.rendered && line.moved_word_width > clip_right - x_origin))) {
        line = find_line_break(font, start, end, x_start, x_origin, clip_right, false, start_state);
    }
    if (line.stop != WalkStop::Sink || line.break_at != start) {
        return line;
    }
    bool forced_rendered = false;
    const auto forced_sink = [&](const Atom& atom) {
        forced_rendered = atom.kind == AtomKind::Glyph || atom.kind == AtomKind::Icon;
        return false;
    };
    const WalkSummary forced = walk_line({font, start, x_start, x_origin}, start, end, forced_sink);
    return {forced.cursor, forced.x - x_start, forced.stop, forced_rendered, start_state};
}

LineBreak next_line_break(const PrintFont& font, std::string_view text, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state) {
    const u8* start = (const u8*)text.data();
    return next_line_break(font, start, start + text.size(), x_start, x_origin, clip_right, word_wrap, start_state);
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
