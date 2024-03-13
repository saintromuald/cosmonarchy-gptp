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
	if (unit && unit->player == *g_local_nation_id) {
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
			add(UnitIndex::from(unit));
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

				if (bw::get_distance_fast(unit->position, other->position) <= distance_quota) {
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
	Vector32 avg_pos = {};
	for (u32 i = 0; i < SelectionCap; ++i) {
		Unit* unit = unit_index[i].get_for_selection(false);
		if (!unit || bw::get_distance_fast(unit->position, anchor->position) > distance_quota) {
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
