#include "unit_search.h"
#include <SCBW\api.h>

#define CELL_SIZE 8
#define CELL_PX_SIZE (CELL_SIZE * 32)
#define CELL_COUNT (256 / CELL_SIZE)

// 8x8 tile cells for up to 256 map size
// NOTE: static will init grid to zeros, and limit scope to this file
// well not really, because unity build, but w/e
static UnitArray unit_grid[CELL_COUNT][CELL_COUNT];

// GRID BASED (current):
// - results in cache misses, because of sparse arrays (fixed size)
// or allocation fracturing (malloc)
// - has potential to result in fewer distance checks,
// especially useful for radial, unfortunately it only is the case in
// mostly niche cases, when cell size plays well with radius basically
// 
// having like 4 size would be ideal for usual finder procs, problem is
// it would eat so much memory, it probably wouldn't be worth it
// performance wise

// BINARY SEARCHED BASED (unit finder had it, badly implemented though):
// - has to sort whole unit array by X and Y
// - always results in O(n) of distance checks
// - still better than old unit finder, since it would sort only once per frame,
// not every finder proc
// - not sure how imperformant array intersection will be

inline void unit_array_add(UnitArray* array, Unit* unit)
{
	array->units[array->size++] = unit;
}

inline void unit_array_remove_fast(UnitArray* array, u32 index)
{
	array->units[index] = array->units[--array->size];
}

void unit_array_remove(UnitArray* array, u32 index)
{
	for (u32 i = array->size - 1; i >= index; i--)
	{
		array->units[i] = array->units[i + 1];
	}
}

bool is_point_in_rect(Point16 point, Box32 rect)
{
	return true
		&& point.x >= rect.left
		&& point.x <= rect.right
		&& point.y >= rect.top
		&& point.y <= rect.bottom;
}

bool is_rect_in_rect(Box32 inner, Box32 outer)
{
	return true
		&& outer.left <= inner.left
		&& inner.right <= outer.right
		&& outer.top <= inner.top
		&& inner.bottom <= outer.bottom;
}

UnitArray search_rect(Box32 rect)
{
	rect = scbw::clampRect(rect);
	UnitArray unit_array;
	unit_array.size = 0;

	const u32 x1 = rect.left / CELL_PX_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_PX_SIZE);
	const u32 y1 = rect.top / CELL_PX_SIZE;
	const u32 y2 = ceil(rect.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is entirely covered by the area
			const Box32 tile_rect =
			{
				x * CELL_PX_SIZE,
				y * CELL_PX_SIZE,
				x * CELL_PX_SIZE + CELL_PX_SIZE,
				y * CELL_PX_SIZE + CELL_PX_SIZE
			};
			const bool tileIsFullyCovered = is_rect_in_rect(tile_rect, rect);

			// check which units are in the area
			UnitArray* current_array = &unit_grid[x][y];
			for (u32 i = 0; i < current_array->size; i++)
			{
				Unit* unit = current_array->units[i];
				if (!tileIsFullyCovered && !is_point_in_rect(unit->position, rect))
				{
					continue;
				}
				unit_array_add(&unit_array, unit);
			}
		}
	}

	return unit_array;
}

UnitArray search_square(Point16 center, u32 radius)
{
	return search_rect({
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	});
}

UnitArray search_radial(Point16 center, u32 radius)
{
	UnitArray unit_array;
	unit_array.size = 0;

	Box32 dims =
	{
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

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is entirely covered by the area
			const Point16 tile_center =
			{
				x * CELL_PX_SIZE + CELL_PX_SIZE / 2,
				y * CELL_PX_SIZE + CELL_PX_SIZE / 2
			};
			const bool tileIsFullyCovered = scbw::getDistanceFast(center, tile_center) + CELL_PX_SIZE * 1.414 <= radius;

			// check which units are in the area
			UnitArray* current_array = &unit_grid[x][y];
			for (u32 i = 0; i < current_array->size; i++)
			{
				Unit* unit = current_array->units[i];
				if (!tileIsFullyCovered && scbw::getDistanceFast(center, unit->position) > radius)
				{
					continue;
				}
				unit_array_add(&unit_array, unit);
			}
		}
	}

	return unit_array;
}

void unit_array_filter(UnitArray* unit_array, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids)
{
	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		if (!unit_flag_match(unit, center, flags, allowed_unit_ids))
		{
			unit_array_remove_fast(unit_array, i--);
			continue;
		}
	}
}

void unit_array_custom_filter(UnitArray* unit_array, SearchFilterFunc filter)
{
	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		if (!filter(unit))
		{
			unit_array_remove_fast(unit_array, i--);
			continue;
		}
	}
}

void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func)
{
	// not a fan of std::sort, probably should replace with own qsort
	// qsort from C can't work with lambdas
	std::sort(unit_array->units, unit_array->units + unit_array->size, [&](Unit* a, Unit* b)
	{
		return score_func(a) > score_func(b);	
	});
}

void unit_array_sort_by_distance(UnitArray* unit_array, Point16 center)
{
	u32* distances = (u32*)malloc(unit_array->size * sizeof(u32));

	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		distances[i] = (u32)scbw::getDistanceFast(center, unit->position);
	}

	std::sort(unit_array->units, unit_array->units + unit_array->size, [&](Unit*& a, Unit*& b)
	{
		int index_a = &a - unit_array->units;
		int index_b = &b - unit_array->units;

		return distances[index_a] > distances[index_b];	
	});

	free(distances);
}

Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func)
{
	u32 best_score;
	Unit* best_unit = nullptr;

	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* new_unit = unit_array->units[i];
		u32 new_score = score_func(new_unit);
		if (!best_unit || new_score > best_score)
		{
			best_unit = new_unit;
			best_score = new_score;
		}
	}

	return best_unit;
}

Unit* unit_array_get_first(UnitArray* unit_array, SearchFilterFunc match_func)
{
	if (!match_func)
	{
		if (unit_array->size)
		{
			return unit_array->units[0];
		}
		return nullptr;
	}

	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		if (match_func(unit))
		{
			return unit;
		}
	}
	return nullptr;
}

void update_unit_grid()
{
	// Reset
	for (u32 x = 0; x < CELL_COUNT; x++)
	{
		for (u32 y = 0; y < CELL_COUNT; y++)
		{
			UnitArray* array = &unit_grid[x][y];
			array->size = 0;
		}
	}

	// Update
	for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
	{
		u32 x = unit->position.x / 32 / CELL_SIZE;
		u32 y = unit->position.y / 32 / CELL_SIZE;
		UnitArray* array = &unit_grid[x][y];
		unit_array_add(array, unit);
	}
}

bool unit_flag_match(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids)
{
	if (!(flags & SearchFlagsIncludingSelf) && unit == center)
	{
		return false;
	}
	if (flags & SearchFlagsNotSelf && unit == center)
	{
		return false;
	}
	if (!(flags & SearchFlagsInvincible) && unit->status & UnitStatus::Invincible)
	{
		return false;
	}
	if (flags & SearchFlagsNoAir && unit->status & UnitStatus::InAir)
	{
		return false;
	}
	if (flags & SearchFlagsNoGround && !(unit->status & UnitStatus::InAir))
	{
		return false;
	}
	if (flags & SearchFlagsSpellcaster)
	{
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
		{
			return false;
		}
	}
	if (flags & SearchFlagsHasEnergy)
	{
		if (unit->energy < 256)
		{
			return false;
		}
	}
	if (flags & SearchFlagsShielded)
	{
		if (!units_dat::ShieldsEnabled[unit->id])
		{
			return false;
		}
		if (unit->shields < 256)
		{
			return false;
		}
	}
	if (flags & SearchFlagsPowered)
	{
		if (unit->status & UnitStatus::DoodadStatesThing)
		{
			return false;
		}
	}
	if (flags & SearchFlagsCompleted)
	{
		if (!(unit->status & UnitStatus::Completed))
		{
			return false;
		}
	}
	if (flags & SearchFlagsNoTrap)
	{
		if (scbw::unitIsTrap(unit))
		{
			return false;
		}
	}
	if (flags & SearchFlagsNoHallucination)
	{
		if (unit->status & UnitStatus::IsHallucination)
		{
			return false;
		}
	}
	if (center)
	{
		if (flags & SearchFlagsFrontalArc)
		{
			u16 angle = scbw::getAngle(center->position.x, center->position.y,
				unit->position.x, unit->position.y);
			u16 unit_angle = center->velocityDirection1;
			s32 arc = (f64)45 / 1.40625;
			// 90 degrees
			s32 delta = unit_angle - angle;
			delta = abs((delta + 128) % 256 - 128);
			if (delta > arc)
			{
				return false;
			}
		}
		if (flags & SearchFlagsOwn && unit->playerId != center->playerId)
		{
			return false;
		}
		if (scbw::isAlliedTo(unit->playerId, center->playerId) && flags & SearchFlagsEnemy)
		{
			return false;
		}
		if (!scbw::isAlliedTo(unit->playerId, center->playerId) && flags & SearchFlagsAlly)
		{
			return false;
		}
	}
	if (allowed_unit_ids.find(unit->id) != allowed_unit_ids.end())
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && flags & SearchFlagsNoOrganic)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && flags & SearchFlagsOnlyOrganic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && flags & SearchFlagsNoMechanical)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && flags & SearchFlagsOnlyMechanic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && flags & SearchFlagsNoBuilding)
	{
		return false;
	}
	if (!(scbw::isEnabled(unit)) && flags & SearchFlagsEnabled)
	{
		return false;
	}
	if (flags & SearchFlagsCanAttackOrCast && !unit->hasWeapon() && !(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
	{
		return false;
	}
	if (flags & SearchFlagsNotWorker && units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
	{
		return false;
	}
	return true;
}
