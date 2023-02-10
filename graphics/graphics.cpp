#include <cassert>
#include "graphics.h"
#include "graphics_errors.h"
#include "graphics_misc.h"
#include "Shape.h"

namespace graphics {

	const s32 MAX_SHAPES  = 5 * 10000; // Increase original limits by 5 times to allow more drawing on screen
	const s32 MAX_STRINGS = 5 * 2000;  //																	-Veeq7

	// Static arrays for fast drawing
	Shape shapes[MAX_SHAPES];
	std::string strings[MAX_STRINGS];
	s32 shape_count = 0;
	s32 string_count = 0;

	void reset_all_graphics() {
		shape_count = 0;
		string_count = 0;
		clear_errors();
	}


	void draw_text(s32 x, s32 y, const std::string& str, FontSize fontSize, CoordType ct) {
		if (string_count >= MAX_STRINGS) {
			set_error(ERR_TOO_MANY_STRINGS);
			return;
		}
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		strings[string_count] = str;
		shapes[shape_count++].set_text(x, y, string_count++, fontSize, ct);
	}

	void draw_dot(s32 x, s32 y, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_dot(x, y, color, ct);
	}

	void draw_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_line(x1, y1, x2, y2, color, ct);
	}

	void draw_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_box(left, top, right, bottom, color, ct);
	}

	void draw_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_circle(x, y, radius, color, ct);
	}

	void draw_filled_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_filled_box(left, top, right, bottom, color, ct);
	}

	void draw_filled_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_filled_circle(x, y, radius, color, ct);
	}

	void draw_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_ellipse(left, top, right, bottom, color, ct);
	}

	void draw_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_dotted_line(x1, y1, x2, y2, color, ct);
	}

	void draw_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
		if (shape_count >= MAX_SHAPES) {
			set_error(ERR_TOO_MANY_SHAPES);
			return;
		}
		shapes[shape_count++].set_dotted_elipse(left, top, right, bottom, color, ct);
	}

	const std::string& get_string_from_index(s32 stringIndex) {
		assert(0 <= stringIndex && stringIndex < MAX_STRINGS);
		return strings[stringIndex];
	}

	s32 draw_all_shapes() {
		s32 drawn_shape_count = 0;
		for (s32 i = 0; i < shape_count; ++i) {
			shapes[i].draw();
		}
		draw_error_messages();
		return shape_count;
	}
}