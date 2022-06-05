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

struct EntryArray {
	u32 size;
	void* entries[EntryLimit];
};

struct SearchIndex {
	u32 size;
	SearchEntry entries[EntryLimit];
	SearchEntry* grid[CELL_COUNT][CELL_COUNT];
};

static SearchIndex unit_index;
static SearchIndex corpse_index;

static void entry_array_add(EntryArray* array, void* data) {
	if (array->size < EntryLimit) {
		array->entries[array->size++] = data;
	}
}

static void entry_array_remove(EntryArray* array, u32 index) {
	if (index < array->size) {
		array->entries[index] = array->entries[--array->size];
	}
}

void unit_array_add(UnitArray* array, Unit* unit) {
	if (array->size < EntryLimit) {
		array->units[array->size++] = unit;
	}
}

void unit_array_remove(UnitArray* array, u32 index) {
	if (index < array->size) {
		array->units[index] = array->units[--array->size];
	}
}

void corpse_array_add(CorpseArray* array, Thingy* corpse) {
	if (array->size < EntryLimit) {
		array->corpses[array->size++] = corpse;
	}
}

void corpse_array_remove(CorpseArray* array, u32 index) {
	if (index < array->size) {
		array->corpses[index] = array->corpses[--array->size];
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
				bool is_fully_covered = false; // TODO: readd this
				if (is_fully_covered || is_point_in_rect(entry->position, rect) && entry_array.size < EntryLimit) {
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
			bool is_fully_covered = false; // TODO: readd this
			for (SearchEntry* entry = index->grid[x][y]; entry; entry = entry->next) {
				if (is_fully_covered || scbw::get_distance_fast(center, entry->position) <= radius) {
					entry_array_add(&entry_array, entry->data);
				}
			}
		}
	}

	return entry_array;
}

inline UnitArray entry_array_to_unit_array(EntryArray entry_array) {
	UnitArray unit_array = *(UnitArray*)&entry_array;
	for (u32 i = 0; i < unit_array.size; ++i) {
		Unit* unit = unit_array.units[i];
		if (unit->is_dead()) {
			unit_array_remove(&unit_array, i--);
		}
	}
	return unit_array;
}

UnitArray unit_search_rect(Box32 rect) {
	return entry_array_to_unit_array(index_search_rect(&unit_index, rect));
}

UnitArray unit_search_square(Point16 center, u32 radius) {
	return entry_array_to_unit_array(index_search_square(&unit_index, center, radius));
}

UnitArray unit_search_radial(Point16 center, u32 radius) {
	return entry_array_to_unit_array(index_search_radial(&unit_index, center, radius));
}

inline CorpseArray entry_array_to_corpse_array(EntryArray entry_array) {
	CorpseArray corpse_array = *(CorpseArray*)&entry_array;
	for (u32 i = 0; i < corpse_array.size; ++i) {
		Thingy* corpse = corpse_array.corpses[i];
		if (corpse->sprite->flags & SpriteFlags::Flag04) {
			corpse_array_remove(&corpse_array, i--);
		}
	}
	return corpse_array;
}

CorpseArray corpse_search_rect(Box32 rect) {
	return entry_array_to_corpse_array(index_search_rect(&corpse_index, rect));
}

CorpseArray corpse_search_square(Point16 center, u32 radius) {
	return entry_array_to_corpse_array(index_search_square(&corpse_index, center, radius));
}

CorpseArray corpse_search_radial(Point16 center, u32 radius) {
	return entry_array_to_corpse_array(index_search_radial(&corpse_index, center, radius));
}

void unit_search_filter(UnitArray* unit_array, Unit* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids) {
	for (u32 i = 0; i < unit_array->size; i++) {
		Unit* unit = unit_array->units[i];
		if (!unit_flag_match(unit, reference, flags, allowed_unit_ids)) {
			unit_array_remove(unit_array, i--);
			continue;
		}
	}
}

void unit_search_custom_filter(UnitArray* unit_array, SearchFilterFunc filter) {
	for (u32 i = 0; i < unit_array->size; i++) {
		Unit* unit = unit_array->units[i];
		if (!filter(unit)) {
			unit_array_remove(unit_array, i--);
			continue;
		}
	}
}

void unit_search_sort(UnitArray* unit_array, SearchScoreFunc score_func) {
	// not a fan of std::sort, probably should replace with own qsort
	// qsort from C can't work with lambdas
	std::sort(unit_array->units, unit_array->units + unit_array->size, [&](Unit* a, Unit* b) {
		return score_func(a) > score_func(b);	
	});
}

static Point16 temp_center;
static s32 distance_compare_func(void const* a, void const* b) {
	return scbw::get_distance_fast(temp_center, ((Unit*)a)->position) > scbw::get_distance_fast(temp_center, ((Unit*)b)->position);	
}

void unit_search_sort_by_distance(UnitArray* unit_array, Point16 center) {
	temp_center = center;
	qsort(unit_array->units, unit_array->size, sizeof(Unit*), distance_compare_func);
}

Unit* unit_search_find_best(UnitArray* unit_array, SearchScoreFunc score_func) {
	s32 best_score = ScoreInvalidUnit;
	Unit* best_unit = 0;

	for (u32 i = 0; i < unit_array->size; i++) {
		Unit* unit = unit_array->units[i];
		s32 score = score_func(unit);
		if (score > best_score) {
			best_unit = unit;
			best_score = score;
		}
	}

	return best_unit;
}

Unit* unit_search_get_first(UnitArray* unit_array, SearchFilterFunc match_func) {
	if (!match_func) {
		if (unit_array->size) {
			return unit_array->units[0];
		}
		return 0;
	}

	for (u32 i = 0; i < unit_array->size; i++) {
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
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
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
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && flags & FilterNoOrganic) {
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && flags & FilterOnlyOrganic) {
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && flags & FilterNoMechanical) {
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && flags & FilterOnlyMechanic) {
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && flags & FilterNoBuilding) {
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building) && flags & FilterBuilding) {
		return false;
	}
	if (!(scbw::is_enabled(unit)) && flags & FilterEnabled) {
		return false;
	}
	if (flags & FilterCanAttackOrCast && !unit->has_weapon() && !(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster)) {
		return false;
	}
	if (flags & FilterNotWorker && units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
		return false;
	}
	return true;
}
