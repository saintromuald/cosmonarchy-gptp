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

s32 Graphics::draw_all_shapes() {
	for (const auto& shape : m_shapes) {
		shape.draw(false);
	}

	draw_error_messages();
	return m_shapes.size();
}

const std::string& Graphics::get_statres_string_from_index(s32 index) {
	assert(0 <= index && index < m_statres_strings.size());
	return m_statres_strings[index];
}

void Graphics::reset_stat_res_graphics() {
	m_statres_bars.clear();
	m_statres_boxes.clear();
	m_statres_strings.clear();
}

void Graphics::draw_statres_bar(s32 x1, s32 x2, s32 y, ColorId color) {
	m_statres_bars.emplace_back().set_line(x1, y, x2, y, color, ON_SCREEN);
}

void Graphics::draw_statres_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color) {
	m_statres_boxes.emplace_back().set_filled_box(left, top, right, bottom, color, ON_SCREEN);
}

void Graphics::draw_statres_text(s32 x, s32 y, const std::string& str) {
	size_t string_idx = m_statres_shapes.size();
	m_statres_strings.emplace_back(str);
	m_statres_shapes.emplace_back().set_text(x, y, string_idx, FONT_MEDIUM, ON_SCREEN);
}

s32 Graphics::draw_stat_res_shapes() {
	for (const auto& bar : m_statres_bars) {
		bar.draw(true);
	}

	for (const auto& box : m_statres_boxes) {
		box.draw(true);
	}

	return m_statres_bars.size() + m_statres_boxes.size();
}

}