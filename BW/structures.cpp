#include "structures.h"
#include <BW\api.h>
#include <ranges>

Unit* UnitIndex::get(bool allow_non_standard_units) {
	if (!index) {
		return 0;
	}

	Unit* unit = &g_unit_table[index - 1];
	if (unit->unique_id != unique_id) {
		return 0;
	}
	if (!unit->sprite || unit->sprite->flags & SpriteFlags::Hidden) {
		return 0;
	}
	if (unit->main_order_id == OrderId::Die && unit->main_order_state == 1) {
		return 0;
	}
	if (!allow_non_standard_units && !helper::unit_is_standard_and_movable(unit)) {
		return 0;
	}

	return unit;
}

Unit* UnitIndex::get_for_selection(bool allow_non_standard_units) {
	Unit* unit = get(allow_non_standard_units);
	if (unit && unit->player == g_local_nation_id) {
		return unit;
	}
	return 0;
}

UnitIndex* ControlGroup::find(UnitIndex index) {
	for (u32 i = 0; i < SelectionCap; i++) {
		if (unit_index[i].value == index.value) {
			return &unit_index[i];
		}
	}
	return 0;
}

bool ControlGroup::add(UnitIndex index) {
	if (!index.is_set_for_selection()) return false;
	if (has(index)) return true;
	for (u32 i = 0; i < SelectionCap; i++) {
		if (!unit_index[i].is_set_for_selection()) {
			unit_index[i] = index;
			return true;
		}
	}
	return false;
}

void ControlGroup::clear() {
	for (u32 i = 0; i < SelectionCap; i++) {
		unit_index[i].clear();
	}
}

void ControlGroup::remove(UnitIndex index) {
	UnitIndex* found = find(index);
	if (found) {
		found->clear();
	}
}

void ControlGroup::select(bool play_response) {
	std::vector<Unit*> to_select;
	to_select.reserve(SelectionCap);
	for (u32 i = 0; i < SelectionCap; i++) {
		Unit* unit = unit_index[i].get_for_selection(true);
		if (unit && std::ranges::find(to_select, unit) == to_select.end()) {
			to_select.push_back(unit);
		}
	}
	bw::select(to_select, play_response);
}

void ControlGroup::add_selection() {
	for (u32 i = 0; i < SelectionLimit; i++) {
		Unit* unit = ccmu_extender.new_local_selection_group[i];
		if (unit) {
			add(unit);
		}
	}
}

void ControlGroup::center_camera() {
	u32 total_count = 0;
	Unit* any_unit = 0;
	for (u32 i = 0; i < SelectionCap; ++i) {
		Unit* unit = unit_index[i].get_for_selection(true);
		if (!unit) {
			continue;
		}
		any_unit = unit;
		total_count++;
	}

	if (!total_count) {
		return;
	}
	if (total_count == 1) {
		assert(any_unit);
		bw::move_camera(any_unit->position);
		return;
	}

	const u32 distance_quota = 10 * 32;
	Unit* anchor = 0;
	{
		u32 best_score = 0;
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = unit_index[i].get_for_selection(false);
			if (!unit) {
				continue;
			}

			u32 score = 1;
			for (u32 j = 0; j < SelectionCap; ++j) {
				if (i == j) {
					continue;
				}
				Unit* other = unit_index[j].get_for_selection(total_count <= 1);
				if (!other ) {
					continue;
				}

				if (bw::distance_between(unit->position, other->position) <= distance_quota) {
					score += 1;
				}
			}

			if (score > best_score) {
				best_score = score;
				anchor = unit;
			}
		}
	}

	if (!anchor) {
		return;
	}
	u32 avg_count = 0;
	Vec32 avg_pos = {};
	for (u32 i = 0; i < SelectionCap; ++i) {
		Unit* unit = unit_index[i].get_for_selection(false);
		if (!unit || bw::distance_between(unit->position, anchor->position) > distance_quota) {
			continue;
		}

		avg_count += 1;
		avg_pos.x += unit->position.x;
		avg_pos.y += unit->position.y;
	}

	if (!avg_count) {
		return;
	}
	avg_pos.x /= avg_count;
	avg_pos.y /= avg_count;
	bw::move_camera({(u16)avg_pos.x, (u16)avg_pos.y});
}

DialogBounds::DialogBounds() : m_left{0}, m_top{0}, m_right{0}, m_bottom{0}, m_width{0}, m_height{0} {
}

DialogBounds DialogBounds::from_ltrb(s16 left, s16 top, s16 right, s16 bottom) {
	DialogBounds db;
	db.m_left = left;
	db.m_top = top;
	db.m_right = right;
	db.m_bottom = bottom;
	db.update_size();
	return db;
}

DialogBounds DialogBounds::from_xywh(s16 x, s16 y, u16 width, u16 height) {
	DialogBounds db;
	db.m_left = x;
	db.m_top = y;
	db.m_width = width;
	db.m_height = height;
	db.m_right = x + static_cast<s16>(width) - 1;
	db.m_bottom = y + static_cast<s16>(height) - 1;
	return db;
}

DialogBounds DialogBounds::from_bounds_ltrb(const Bounds bounds) {
	return from_ltrb(bounds.left, bounds.top, bounds.right, bounds.bottom);
}

DialogBounds DialogBounds::from_bounds_xywh(const Bounds bounds){
	return from_xywh(bounds.x, bounds.y, bounds.w, bounds.h);
}

void DialogBounds::set_ltrb(s16 left, s16 top, s16 right, s16 bottom) {
	m_left = left;
	m_top = top;
	m_right = right;
	m_bottom = bottom;
	update_size();
}

void DialogBounds::set_xywh(s16 x, s16 y, u16 width, u16 height) {
	m_left = x;
	m_top = y;
	m_width = width;
	m_height = height;
	m_right = x + static_cast<s16>(width) - 1;
	m_bottom = y + static_cast<s16>(height) - 1;
}

bool DialogBounds::contains(const Point16 point) const {
	return point.x >= static_cast<u16>(m_left) && point.x <= static_cast<u16>(m_right) &&
		point.y >= static_cast<u16>(m_top) && point.y <= static_cast<u16>(m_bottom);
}

bool DialogBounds::contains(u16 x, u16 y) const {
	return contains(Point16{x, y});
}

bool DialogBounds::contains_mouse() const {
	return contains(static_cast<u16>(g_mouse->x), static_cast<u16>(g_mouse->y));
}

bool DialogBounds::is_empty() const {
	return m_width == 0 || m_height == 0;
}

bool DialogBounds::is_valid() const {
	return m_right >= m_left && m_bottom >= m_top;
}

Point16 DialogBounds::center() const {
	return Point16{
		static_cast<u16>(m_left + static_cast<s16>(m_width) / 2),
		static_cast<u16>(m_top + static_cast<s16>(m_height) / 2)
	};
}

Point16 DialogBounds::top_left() const {
	return Point16{static_cast<u16>(m_left), static_cast<u16>(m_top)};
}

Point16 DialogBounds::top_right() const {
	return Point16{static_cast<u16>(m_right), static_cast<u16>(m_top)};
}

Point16 DialogBounds::bottom_left() const {
	return Point16{static_cast<u16>(m_left), static_cast<u16>(m_bottom)};
}

Point16 DialogBounds::bottom_right() const {
	return Point16{static_cast<u16>(m_right), static_cast<u16>(m_bottom)};
}

DialogBounds DialogBounds::intersect(const DialogBounds& other) const {
	s16 new_left = std::max(m_left, other.m_left);
	s16 new_top = std::max(m_top, other.m_top);
	s16 new_right = std::min(m_right, other.m_right);
	s16 new_bottom = std::min(m_bottom, other.m_bottom);

	if (new_right < new_left || new_bottom < new_top) {
		return DialogBounds{};
	}

	return from_ltrb(new_left, new_top, new_right, new_bottom);
}

DialogBounds DialogBounds::unite(const DialogBounds& other) const {
	if (is_empty()) return other;
	if (other.is_empty()) return *this;

	return from_ltrb(
		std::min(m_left, other.m_left),
		std::min(m_top, other.m_top),
		std::max(m_right, other.m_right),
		std::max(m_bottom, other.m_bottom)
	);
}

DialogBounds DialogBounds::inflate(s16 dx, s16 dy) const {
	return from_ltrb(m_left - dx, m_top - dy, m_right + dx, m_bottom + dy);
}

DialogBounds DialogBounds::offset(s16 dx, s16 dy) const {
	return from_ltrb(m_left + dx, m_top + dy, m_right + dx, m_bottom + dy);
}

DialogBounds::operator Box16() const {
	return Box16{m_left, m_top, m_right, m_bottom};
}

DialogBounds::operator Box32() const {
	return Box32{m_left, m_top, m_right, m_bottom};
}

DialogBounds::operator Bounds() const {
	Bounds bounds;
	bounds.left = m_left;
	bounds.top = m_top;
	bounds.right = m_right;
	bounds.bottom = m_bottom;
	bounds.width = m_width;
	bounds.height = m_height;
	return bounds;
}

Bounds DialogBounds::to_bounds() const {
	return *this;
}

void DialogBounds::update_size() {
	m_width = static_cast<u16>(std::max(0, m_right - m_left + 1));
	m_height = static_cast<u16>(std::max(0, m_bottom - m_top + 1));
}