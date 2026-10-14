#pragma once
#include "types.h"
#include "walk.h"

#include <cstddef>
#include <string_view>

namespace text {

struct LineBreak {
    const u8* break_at; // the line is [start, break_at); past the '\n' at line ends
    s32 width;
    WalkStop stop;
    u16 carry_color = 0; // carry color across line split
};

// Mirrors font_find_line_break (0x0041F920). Word wrap returns the line start unchanged when the first word alone overflows so callers can retry in char-wrap mode.
LineBreak find_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap);
LineBreak find_line_break(u8 slot, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap);

// Mirrors font_fit_text_to_width (0x0041FC20) the offset snaps to a codepoint boundary.
size_t fit_text_to_width(u8 slot, std::string_view text, s32 max_width);

// Mirrors font_max_line_width (0x0041FC80).
s32 max_line_width(u8 slot, std::string_view text);

s32 cursor_width(const PrintFont& font);

} // namespace text