#pragma once
#include "../types.h"
#include "Shape.h"

#include <vector>
#include <string>

namespace graphics {

class Graphics {
public:
	// Resets all previous graphics and clears error messages. This should be
	// called every frame in game_hooks.cpp.
	void reset_all_graphics();

	void draw_text(s32 x, s32 y, const std::string& str, FontSize fontSize = FONT_MEDIUM, CoordType ct = ON_SCREEN);
	void draw_dot(s32 x, s32 y, ColorId color, CoordType ct = ON_SCREEN);
	void draw_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct = ON_SCREEN);
	void draw_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct = ON_SCREEN);
	void draw_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct = ON_SCREEN);
	void draw_filled_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct = ON_SCREEN);
	void draw_filled_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct = ON_SCREEN);
	void draw_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct = ON_SCREEN);
	void draw_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct = ON_SCREEN);
	void draw_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct = ON_SCREEN);

	// ON_MAP shapes scale with terrain zoom; ON_SCREEN/ON_MOUSE shapes are drawn unscaled after world resample.
	s32 draw_world_shapes();
	s32 draw_screen_shapes();

	const std::string& get_string_from_index(s32 index);

private:
	std::vector<Shape> m_shapes;
	std::vector<std::string> m_strings;
};

}

inline graphics::Graphics g_graphics;