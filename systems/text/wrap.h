#pragma once
#include "types.h"
#include "walk.h"

#include <cstddef>
#include <string_view>

namespace text {

// A color state value: a plain color code (is_color_code), or a player color packed
// as ((row + 1) << 8) | 0x0B with row indexing g_player_in_game_colors.
struct ColorState {
    u16 current = 0;
    u16 saved = 0;
};

u16 pair_state(u8 row);
u8 pair_state_row(u16 state);
bool is_pair_state(u16 state);

enum class ColorStep : u8 {
    None,
    Push,
    Revert,
};

ColorStep color_step(bool grey_current, const Atom& atom);

struct LineBreak {
    const u8* break_at; // the line is [start, break_at); past the '\n' at line ends
    s32 width;
    WalkStop stop;
    bool rendered;    // a glyph or icon fell inside the line
    ColorState carry; // the color state at the break, seeded from the line's start state
    s32 moved_word_width = 0; // Sink: the width of the word that moved to the next line
};

// Mirrors font_find_line_break (0x0041F920). word-wrap rollbacks also report the moved word's width so callers can retry without re-walking it.
LineBreak find_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state = {});

// word wrap with character wrap fallback
LineBreak next_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state = {});
LineBreak next_line_break(const PrintFont& font, std::string_view text, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap, ColorState start_state = {});

// Mirrors font_fit_text_to_width (0x0041FC20) the offset snaps to a codepoint boundary.
size_t fit_text_to_width(u8 slot, std::string_view text, s32 max_width);

// Mirrors font_max_line_width (0x0041FC80).
s32 max_line_width(u8 slot, std::string_view text);

s32 cursor_width(const PrintFont& font);

} // namespace text