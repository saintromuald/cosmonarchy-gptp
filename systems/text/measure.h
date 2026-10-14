#pragma once
#include "types.h"

#include <optional>
#include <string_view>

namespace graphics {
struct Font;
}

namespace text {

struct PrintFont;

// Width of the remaining text on the current line
// x_origin anchors tab stops the same way the rendering walk will.
s32 line_width(u8 slot, std::string_view text, s32 x_start = 0, s32 x_origin = 0);
s32 line_width(const PrintFont& font, std::string_view text, s32 x_start = 0, s32 x_origin = 0);

// Total width across all lines
s32 text_width(u8 slot, std::string_view text);

// Line count times the line advance
s32 text_height(u8 slot, std::string_view text);

// The font_base slot owning the font, or nullopt for fonts outside the slot table (wireframe fonts and the like)
std::optional<u8> slot_of(const graphics::Font* font);

} // namespace text