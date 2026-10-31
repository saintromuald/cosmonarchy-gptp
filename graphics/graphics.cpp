#include <cassert>
#include "graphics.h"
#include "graphics_errors.h"
#include "Shape.h"

namespace graphics {

void Graphics::reset_all_graphics() {
	m_shapes.clear();
	m_strings.clear();
	clear_errors();
}

void Graphics::draw_text(s32 x, s32 y, const std::string& str, FontSize fontSize, CoordType ct) {
	size_t string_idx = m_strings.size();
	m_strings.emplace_back(str);
	m_shapes.emplace_back().set_text(x, y, string_idx, fontSize, ct);
}

void Graphics::draw_dot(s32 x, s32 y, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_dot(x, y, color, ct);
}

void Graphics::draw_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_line(x1, y1, x2, y2, color, ct);
}

void Graphics::draw_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_box(left, top, right, bottom, color, ct);
}

void Graphics::draw_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_circle(x, y, radius, color, ct);
}

void Graphics::draw_filled_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_filled_box(left, top, right, bottom, color, ct);
}

void Graphics::draw_filled_circle(s32 x, s32 y, s32 radius, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_filled_circle(x, y, radius, color, ct);
}

void Graphics::draw_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_ellipse(left, top, right, bottom, color, ct);
}

void Graphics::draw_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_dotted_line(x1, y1, x2, y2, color, ct);
}

void Graphics::draw_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color, CoordType ct) {
	m_shapes.emplace_back().set_dotted_elipse(left, top, right, bottom, color, ct);
}

const std::string& Graphics::get_string_from_index(s32 index) {
	assert(0 <= index && index < m_strings.size());
	return m_strings[index];
}

s32 Graphics::draw_world_shapes() {
	s32 drawn = 0;
	for (const auto& shape : m_shapes) {
		if (shape.coordType == ON_MAP) {
			shape.draw();
			++drawn;
		}
	}

	return drawn;
}

s32 Graphics::draw_screen_shapes() {
	s32 drawn = 0;
	for (const auto& shape : m_shapes) {
		if (shape.coordType != ON_MAP) {
			shape.draw();
			++drawn;
		}
	}

	draw_error_messages();
	return drawn;
}

}
// ---------------------------------------------------------------------------
// Exported for Cosmonarchy Plus: screen-fixed overlay text/shapes through the
// fork's graphics system. The shapes draw post-resample in
// draw_deferred_shapes (present_frame, i==2) and the fork's refresh handling
// (draw_hook.cpp: want_refresh -> refresh_screen) presents them — the same
// pipeline the fork's own UI text uses. Coordinates are plain screen pixels
// on the presented buffer (ON_SCREEN).
// ---------------------------------------------------------------------------

extern "C" __declspec(dllexport) void cosmo_plus_gfx_text(int x, int y, const char* text) {
    g_graphics.draw_text(x, y, text ? text : "", graphics::FONT_MEDIUM, graphics::ON_SCREEN);
}

extern "C" __declspec(dllexport) void cosmo_plus_gfx_box(int left, int top, int right, int bottom, unsigned char color) {
    g_graphics.draw_box(left, top, right, bottom, color, graphics::ON_SCREEN);
}

extern "C" __declspec(dllexport) void cosmo_plus_gfx_filled_box(int left, int top, int right, int bottom, unsigned char color) {
    g_graphics.draw_filled_box(left, top, right, bottom, color, graphics::ON_SCREEN);
}
