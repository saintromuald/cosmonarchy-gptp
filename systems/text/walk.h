#pragma once
#include "types.h"
#include "BW/enumerations.h"
#include "decoder.h"
#include "font_stack.h"
#include "shaper.h"
#include "text.h"

#include "systems/graphics/grp.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

namespace text {

enum class AtomKind : u8 {
    Glyph,        // one shaped codepoint (advance includes char spacing)
    Icon,         // inline unit icon
    Cursor,       // the 04+marker pair: the text cursor, zero advance
    Space,
    Tab,          // advanced to the next tab stop from the origin
    Control,      // color/align/no-op control byte: zero width
    PlayerColor,  // a consumed 0B/14 pair: zero width
};

// One step of a line walk.
struct Atom {
    AtomKind kind;
    const u8* start;         // first raw byte of the atom
    const u8* next;          // one past the atom's last raw byte
    s32 x;                   // absolute position after the atom
    char32_t codepoint = 0;  // Glyph
    u32 glyph_index = 0;     // Glyph
    FontFace* face = nullptr;// Glyph
    s32 advance = 0;         // full width contribution (Glyph/Icon include spacing)
    s32 offset_x = 0;        // Glyph: shaper offset
    s32 offset_y = 0;        // Glyph: shaper offset
    u8 control = 0;          // Control/PlayerColor: the control byte value
    u8 payload = 0;          // PlayerColor: the consumed pair byte
    u16 unit_id = 0;         // Icon
};

enum class WalkStop : u8 {
    End,      // input exhausted
    Newline,  // '\n' consumed
    PrintEnd, // 0x00 or 0x0C consumed
    Sink,     // the sink returned false after an atom
};

struct WalkSummary {
    WalkStop stop;
    const u8* cursor;    // first unconsumed byte
    const u8* stop_byte; // the stopping token's first byte (== cursor at End)
    s32 x;               // x after the last consumed atom
};

struct WalkInput {
    PrintFont font;
    const u8* start;   // start of the decoded range (cursor pair lookback boundary)
    s32 x;             // x at walk start
    s32 x_origin;      // tab stops are multiples of the tab width from here
};


inline s32 tab_stop(s32 width, s32 x, s32 origin) {
    if (width <= 0) return 0;
    return width - (x - origin) % width;
}

// Walks one line of atoms; the sink returns false to stop after that atom.
template <typename Sink>
WalkSummary walk_line(const WalkInput& input, const u8* cursor, const u8* end, Sink&& sink) {
    TextSystem& text_system = TextSystem::instance();
    const PrintFont& font = input.font;
    FontStack& stack = text_system.stack(font.stack);
    Shaper& shaper = text_system.shaper();
    const s32 spacing = font.char_spacing;
    const s32 space_advance = font.space_advance;
    const s32 tab_width = font.tab_width;

    s32 x = input.x;
    auto& [run_codepoints, run_starts, shaped] = text_system.walk_scratch();
    run_codepoints.clear();
    run_starts.clear();
    FontFace* run_face = nullptr;
    const u8* run_end = cursor;

    // Returns the stopped atom when the sink halts mid-run, else nullopt.
    const auto flush_run = [&]() -> std::optional<Atom> {
        assert(run_codepoints.empty() || run_face);
        if (!run_codepoints.empty() && run_face) {
            shaper.shape_into(shaped, *run_face, font.pixel_height, run_codepoints);
            assert(shaped.size() == run_starts.size()); // the shaper is 1 codepoint -> 1 glyph
            const size_t count = std::min(shaped.size(), run_starts.size());
            for (size_t i = 0; i < count; ++i) {
                const s32 advance = shaped[i].advance + spacing;
                x += advance;
                Atom atom{
                    .kind = AtomKind::Glyph,
                    .start = run_starts[i],
                    .next = i + 1 < run_starts.size() ? run_starts[i + 1] : run_end,
                    .x = x,
                    .codepoint = run_codepoints[i],
                    .glyph_index = shaped[i].glyph_index,
                    .face = run_face,
                    .advance = advance,
                    .offset_x = shaped[i].offset_x,
                    .offset_y = shaped[i].offset_y,
                };
                if (!sink(atom)) {
                    run_codepoints.clear();
                    run_starts.clear();
                    return atom;
                }
            }
        }
        run_codepoints.clear();
        run_starts.clear();
        return std::nullopt;
    };

    const auto sink_stop = [&](const Atom& atom) -> WalkSummary {
        return {WalkStop::Sink, atom.next, atom.start, x};
    };

    while (cursor != end) { // decode_next only returns nullopt at end
        const u8* token_start = cursor;
        if (!run_starts.empty()) {
            run_end = token_start;
        }
        const auto token = decode_next(cursor, end);
        assert(token);

        if (const auto* codepoint = std::get_if<Codepoint>(&*token)) {
            if (is_cursor_pair(token_start, cursor, input.start)) {
                if (const auto stopped = flush_run()) {
                    return {WalkStop::Sink, stopped->next, stopped->start, x};
                }
                const Atom cursor_atom{.kind = AtomKind::Cursor, .start = token_start, .next = cursor, .x = x};
                if (!sink(cursor_atom)) {
                    return {WalkStop::Sink, cursor_atom.next, cursor_atom.start, x};
                }
                continue;
            }
            FontFace* face = stack.resolve(codepoint->value, run_face);
            if (face != run_face) {
                if (const auto stopped = flush_run()) {
                    return sink_stop(*stopped);
                }
                run_face = face;
            }
            run_codepoints.push_back(codepoint->value);
            run_starts.push_back(token_start);
            continue;
        }
        if (const auto stopped = flush_run()) {
            return sink_stop(*stopped);
        }
        if (const auto* control = std::get_if<Control>(&*token)) {
            switch (control->value) {
                case '\n':
                    return {WalkStop::Newline, cursor, token_start, x};
                case '\0':
                case '\f':
                    return {WalkStop::PrintEnd, cursor, token_start, x};
                case '\t': {
                    const s32 advance = tab_stop(tab_width, x, input.x_origin);
                    x += advance;
                    const Atom atom{.kind = AtomKind::Tab, .start = token_start, .next = cursor, .x = x, .advance = advance};
                    if (!sink(atom)) {
                        return sink_stop(atom);
                    }
                } continue;
                case ' ': {
                    x += space_advance;
                    const Atom atom{.kind = AtomKind::Space, .start = token_start, .next = cursor, .x = x, .advance = space_advance};
                    if (!sink(atom)) {
                        return sink_stop(atom);
                    }
                } continue;
                case std::to_underlying(TextColor::PlayerColor):
                case std::to_underlying(TextColor::PlayerColor2): {
                    u8 payload = 0;
                    if (cursor != end) {
                        const u8 next = *cursor;
                        if ((next >= 1 && next <= 8) || (next >= 0x0E && next <= 0x11)) {
                            payload = next;
                            ++cursor; // valid pair; the payload byte renders nothing
                        }
                    }
                    const Atom atom{.kind = AtomKind::PlayerColor, .start = token_start, .next = cursor, .x = x, .control = control->value, .payload = payload};
                    if (!sink(atom)) {
                        return sink_stop(atom);
                    }
                } continue;
                default: { // colors, in-band aligns, no-ops: zero width
                    const Atom atom{.kind = AtomKind::Control, .start = token_start, .next = cursor, .x = x, .control = control->value};
                    if (!sink(atom)) {
                        return sink_stop(atom);
                    }
                } continue;
            }
        }
        const auto* icon = std::get_if<Icon>(&*token);
        const s32 advance = graphics::unit_icon_advance(icon->unit_id, font.pixel_height, spacing);
        x += advance;
        const Atom atom{.kind = AtomKind::Icon, .start = token_start, .next = cursor, .x = x, .advance = advance, .unit_id = icon->unit_id};
        if (!sink(atom)) {
            return sink_stop(atom);
        }
    }
    if (!run_starts.empty()) {
        run_end = cursor;
    }
    if (const auto stopped = flush_run()) {
        return sink_stop(*stopped);
    }
    return {WalkStop::End, end, end, x};
}

} // namespace text
