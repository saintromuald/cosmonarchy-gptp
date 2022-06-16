#include "unit_search.h"
#include <SCBW\api.h>

#define CELL_SIZE 8
#define CELL_PX_SIZE (CELL_SIZE * 32)
#define CELL_COUNT (256 / CELL_SIZE)

struct SearchEntry {
	SearchEntry* next;
	Point16 position;
	void* data;
};

struct SearchIndex {
	u32 size;
	SearchEntry entries[EntryLimit];
	SearchEntry* grid[CELL_COUNT][CELL_COUNT];
};

static SearchIndex unit_index;
static SearchIndex corpse_index;
static SearchIndex effect_index;

void entry_array_add(EntryArray* array, void* data) {
	if (array->count < EntryLimit) {
		array->entries[array->count++] = data;
	}
}

void entry_array_remove(EntryArray* array, u32 index) {
	if (index < array->count) {
		array->entries[index] = array->entries[--array->count];
	}
}

bool is_point_in_rect(Point16 point, Box32 rect)
{
	return point.x >= rect.left
		&& point.x <= rect.right
		&& point.y >= rect.top
		&& point.y <= rect.bottom;
}

bool is_rect_in_rect(Box32 inner, Box32 outer)
{
	return outer.left <= inner.left
		&& inner.right <= outer.right
		&& outer.top <= inner.top
		&& inner.bottom <= outer.bottom;
}

static void search_index_reset(SearchIndex* index) {
	memset(index, 0, sizeof(SearchIndex));
}

static void search_index_add(SearchIndex* index, Point16 position, void* data) {
	if (index->size < EntryLimit) {
		SearchEntry* entry = &index->entries[index->size++];
		entry->position = position;
		entry->data = data;

		u32 x = position.x / CELL_PX_SIZE;
		u32 y = position.y / CELL_PX_SIZE;
		entry->next = index->grid[x][y];
		index->grid[x][y] = entry;
	}
}

static EntryArray index_search_rect(SearchIndex* index, Box32 rect) {
	rect = scbw::clampRect(rect);
	EntryArray entry_array = {};

	const u32 x1 = rect.left / CELL_PX_SIZE;
	const u32 y1 = rect.top / CELL_PX_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(rect.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++) {
		for (u32 y = y1; y < y2; y++) {
			for (SearchEntry* entry = index->grid[x][y]; entry; entry = entry->next) {
				Box32 tile_rect = {
					x * CELL_PX_SIZE,
					y * CELL_PX_SIZE,
					x * CELL_PX_SIZE + CELL_PX_SIZE,
					y * CELL_PX_SIZE + CELL_PX_SIZE
				};
				bool is_fully_covered = is_rect_in_rect(tile_rect, rect);

				if (is_fully_covered || is_point_in_rect(entry->position, rect) && entry_array.count < EntryLimit) {
					entry_array_add(&entry_array, entry->data);
				}
			}
		}
	}

	return entry_array;
}

static EntryArray index_search_square(SearchIndex* index, Point16 center, u32 radius) {
	return index_search_rect(index, {
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	});
}

static EntryArray index_search_radial(SearchIndex* index, Point16 center, u32 radius) {
	EntryArray entry_array = {};

	Box32 dims = {
		center.x - radius,
		center.y - radius,
		center.x + radius,
		center.y + radius
	};
	dims = scbw::clampRect(dims);

	const u32 x1 = dims.left / CELL_PX_SIZE;
	const u32 y1 = dims.top / CELL_PX_SIZE;
	const u32 x2 = ceil(dims.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(dims.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++) {
		for (u32 y = y1; y < y2; y++) {
			Point16 tile_center = {
				x * CELL_PX_SIZE + CELL_PX_SIZE / 2,
				y * CELL_PX_SIZE + CELL_PX_SIZE / 2
			};
			bool is_fully_covered = scbw::get_distance_fast(center, tile_center) + CELL_PX_SIZE * 1.414 <= radius;

			for (SearchEntry* entry = index->grid[x][y]; entry; entry = entry->next) {
				if (is_fully_covered || scbw::get_distance_fast(center, entry->position) <= radius) {
					entry_array_add(&entry_array, entry->data);
				}
			}
		}
	}

	return entry_array;
}

inline EntryArray filter_unit_array(EntryArray entry_array) {
	for (u32 i = 0; i < entry_array.count; ++i) {
		Unit* unit = entry_array.units[i];
		if (unit->is_dead()) {
			entry_array_remove(&entry_array, i--);
		}
	}
	return entry_array;
}

EntryArray unit_search_rect(Box32 rect) {
	return filter_unit_array(index_search_rect(&unit_index, rect));
}

EntryArray unit_search_square(Point16 center, u32 radius) {
	return filter_unit_array(index_search_square(&unit_index, center, radius));
}

EntryArray unit_search_radial(Point16 center, u32 radius) {
	return filter_unit_array(index_search_radial(&unit_index, center, radius));
}

inline EntryArray filter_corpse_array(EntryArray entry_array) {
	for (u32 i = 0; i < entry_array.count; ++i) {
		Thingy* corpse = entry_array.corpses[i];
		if (corpse->sprite->flags & SpriteFlags::Flag04) {
			entry_array_remove(&entry_array, i--);
		}
	}
	return entry_array;
}

EntryArray corpse_search_rect(Box32 rect) {
	return filter_corpse_array(index_search_rect(&corpse_index, rect));
}

EntryArray corpse_search_square(Point16 center, u32 radius) {
	return filter_corpse_array(index_search_square(&corpse_index, center, radius));
}

EntryArray corpse_search_radial(Point16 center, u32 radius) {
	return filter_corpse_array(index_search_radial(&corpse_index, center, radius));
}

EntryArray effect_search_rect(Box32 rect) {
	return index_search_rect(&effect_index, rect);
}

EntryArray effect_search_square(Point16 center, u32 radius) {
	return index_search_square(&effect_index, center, radius);
}

EntryArray effect_search_radial(Point16 center, u32 radius) {
	return index_search_radial(&effect_index, center, radius);
}

void unit_search_filter(EntryArray* unit_array, Unit* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids) {
	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		if (!unit_flag_match(unit, reference, flags, allowed_unit_ids)) {
			entry_array_remove(unit_array, i--);
			continue;
		}
	}
}

void unit_search_custom_filter(EntryArray* unit_array, SearchFilterFunc filter) {
	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		if (!filter(unit)) {
			entry_array_remove(unit_array, i--);
			continue;
		}
	}
}

void unit_search_sort(EntryArray* unit_array, SearchScoreFunc score_func) {
	// not a fan of std::sort, probably should replace with own qsort
	// qsort from C can't work with lambdas
	std::sort(unit_array->units, unit_array->units + unit_array->count, [&](Unit* a, Unit* b) {
		return score_func(a) > score_func(b);	
	});
}

static Point16 temp_center;
static s32 distance_compare_func(void const* a, void const* b) {
	return scbw::get_distance_fast(temp_center, ((Unit*)a)->position) > scbw::get_distance_fast(temp_center, ((Unit*)b)->position);	
}

void unit_search_sort_by_distance(EntryArray* unit_array, Point16 center) {
	temp_center = center;
	qsort(unit_array->units, unit_array->count, sizeof(Unit*), distance_compare_func);
}

Unit* unit_search_find_best(EntryArray* unit_array, SearchScoreFunc score_func) {
	s32 best_score = ScoreInvalidUnit;
	Unit* best_unit = 0;

	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		s32 score = score_func(unit);
		if (score > best_score) {
			best_unit = unit;
			best_score = score;
		}
	}

	return best_unit;
}

Unit* unit_search_get_first(EntryArray* unit_array, SearchFilterFunc match_func) {
	if (!match_func) {
		if (unit_array->count) {
			return unit_array->units[0];
		}
		return 0;
	}

	for (u32 i = 0; i < unit_array->count; i++) {
		Unit* unit = unit_array->units[i];
		if (match_func(unit)) {
			return unit;
		}
	}
	return 0;
}


void update_search_index() {
	// Add units
	search_index_reset(&unit_index);
	for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next) {
		search_index_add(&unit_index, unit->position, unit);
	}

	// Add corpses
	search_index_reset(&corpse_index);
	for (Thingy* thingy = *first_lone_sprite; thingy; thingy = thingy->next) {
		if (thingy->sprite && scbw::sprite_id_is_corpse(thingy->sprite->sprite_id)) {
			search_index_add(&corpse_index, thingy->sprite->position, thingy);
		}
	}

	// Reset effects
	search_index_reset(&effect_index);
}

void update_search_index_effects() {
	// Add effects
	for (Effect& effect : globals.effects) {
		Point16 position = {};
		position.x = effect.position.x;
		position.y = effect.position.y;
		search_index_add(&effect_index, position, &effect);
	}
}

bool unit_flag_match(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids) {
	if (!(flags & FilterIncludingSelf) && unit == center) {
		return false;
	}
	if (flags & FilterNotSelf && unit == center) {
		return false;
	}
	if (!(flags & FilterInvincible) && unit->status & UnitStatus::Invincible) {
		return false;
	}
	if (flags & FilterNoAir && unit->status & UnitStatus::InAir) {
		return false;
	}
	if (flags & FilterNoGround && !(unit->status & UnitStatus::InAir)) {
		return false;
	}
	if (flags & FilterSpellcaster) {
		if (!(units_dat::base_property[unit->id] & UnitProperty::Spellcaster)) {
			return false;
		}
	}
	if (flags & FilterHasEnergy) {
		if (unit->energy < 256) {
			return false;
		}
	}
	if (flags & FilterShielded) {
		if (!units_dat::ShieldsEnabled[unit->id]) {
			return false;
		}
		if (unit->shields < 256) {
			return false;
		}
	}
	if (flags & FilterPowered) {
		if (unit->status & UnitStatus::Frozen) {
			return false;
		}
	}
	if (flags & FilterCompleted) {
		if (!(unit->status & UnitStatus::Completed)) {
			return false;
		}
	}
	if (flags & FilterNoTrap) {
		if (scbw::unit_is_trap(unit)) {
			return false;
		}
	}
	if (flags & FilterNoHallucination) {
		if (unit->status & UnitStatus::IsHallucination) {
			return false;
		}
	}
	if (center) {
		if (flags & FilterFrontalArc) {
			u16 angle = scbw::get_angle(center->position.x, center->position.y,
				unit->position.x, unit->position.y);
			u16 unit_angle = center->velocity_direction_1;
			s32 arc = (f64)45 / 1.40625;
			// 90 degrees
			s32 delta = unit_angle - angle;
			delta = abs((delta + 128) % 256 - 128);
			if (delta > arc) {
				return false;
			}
		}
		if (flags & FilterOwn && unit->player != center->player) {
			return false;
		}
		if (scbw::is_allied_to(unit->player, center->player) && flags & FilterEnemy) {
			return false;
		}
		if (!scbw::is_allied_to(unit->player, center->player) && flags & FilterAlly) {
			return false;
		}
	}
	if (allowed_unit_ids.size() && allowed_unit_ids.find(unit->id) == allowed_unit_ids.end()) {
		return false;
	}
	if (units_dat::base_property[unit->id] & UnitProperty::Organic && flags & FilterNoOrganic) {
		return false;
	}
	if (!(units_dat::base_property[unit->id] & UnitProperty::Organic) && flags & FilterOnlyOrganic) {
		return false;
	}
	if (units_dat::base_property[unit->id] & UnitProperty::Mechanical && flags & FilterNoMechanical) {
		return false;
	}
	if (!(units_dat::base_property[unit->id] & UnitProperty::Mechanical) && flags & FilterOnlyMechanic) {
		return false;
	}
	if (units_dat::base_property[unit->id] & UnitProperty::Building && flags & FilterNoBuilding) {
		return false;
	}
	if (!(units_dat::base_property[unit->id] & UnitProperty::Building) && flags & FilterBuilding) {
		return false;
	}
	if (!(scbw::is_enabled(unit)) && flags & FilterEnabled) {
		return false;
	}
	if (flags & FilterCanAttackOrCast && !unit->has_weapon() && !(units_dat::base_property[unit->id] & UnitProperty::Spellcaster)) {
		return false;
	}
	if (flags & FilterNotWorker && units_dat::base_property[unit->id] & UnitProperty::Worker) {
		return false;
	}
	return true;
}
