#include "unit_search.h"
#include <SCBW\api.h>

#define CELL_SIZE 8
#define CELL_COUNT (256 / CELL_SIZE)

// 8x8 tile cells for up to 256 map size
// NOTE: static will init grid to zeros, and limit scope to this file
// well not really, because unity build, but w/e
static UnitArray unit_grid[CELL_COUNT][CELL_COUNT]; 

bool unit_flag_match(Unit* center, Unit* unit, u32 context, std::vector<u16> allowedUnitIds);

inline void unit_array_add(UnitArray* array, Unit* unit)
{
	array->units[array->count++] = unit;
}

inline void unit_array_remove_fast(UnitArray* array, u32 index)
{
	array->units[index] = array->units[--array->count];
}

void unit_array_remove(UnitArray* array, u32 index)
{
	for (u32 i = array->count - 1; i >= index; i--)
	{
		array->units[i] = array->units[i + 1];
	}
}

void unit_array_combine(UnitArray* target, UnitArray* source)
{
	for (u32 i = 0; i < source->count; i++)
	{
		Unit* unit = source->units[i];
		unit_array_add(target, unit);
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
	unit_array.count = 0;

	const u32 x1 = rect.left / CELL_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_SIZE);
	const u32 y1 = rect.top / CELL_SIZE;
	const u32 y2 = ceil(rect.bottom / (f32)CELL_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is entirely covered by rect
			const Box32 tile_rect =
			{
				x * CELL_SIZE,
				x * CELL_SIZE + CELL_SIZE,
				y * CELL_SIZE,
				y * CELL_SIZE + CELL_SIZE
			};
			const bool tileIsFullyCovered = is_rect_in_rect(tile_rect, rect);

			UnitArray* current_array = &unit_grid[x][y];
			for (u32 i = 0; i < current_array->count; i++)
			{
				Unit* unit = current_array->units[i];
				if (!tileIsFullyCovered || !is_point_in_rect(unit->position, rect))
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
		center.x - (s32)radius / 2,
		center.y - (s32)radius / 2,
		center.x + (s32)radius / 2,
		center.y + (s32)radius / 2
	});
}

UnitArray search_radial(Point16 center, u32 radius)
{
	UnitArray unit_array;
	unit_array.count = 0;

	const u32 x1 = (center.x - radius) / CELL_SIZE;
	const u32 x2 = ceil((center.x + radius) / (f32)CELL_SIZE);
	const u32 y1 = (center.y - radius) / CELL_SIZE;
	const u32 y2 = ceil((center.y + radius) / (f32)CELL_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is entirely covered by rect
			const Point16 tile_center =
			{
				x * CELL_SIZE + CELL_SIZE / 2,
				y * CELL_SIZE + CELL_SIZE / 2
			};
			const bool tileIsFullyCovered = scbw::getDistanceFast(center, tile_center) + CELL_SIZE * 32 * 1.414 <= radius;

			// check if unit is in the rect
			UnitArray* current_array = &unit_grid[x][y];
			for (u32 i = 0; i < current_array->count; i++)
			{
				Unit* unit = current_array->units[i];
				if (!tileIsFullyCovered || scbw::getDistanceFast(center, unit->position) > radius)
				{
					continue;
				}
				unit_array_add(&unit_array, unit);
			}
		}
	}

	return unit_array;
}

void unit_array_filter(UnitArray* unit_array, u32 flags, SearchFilterFunc filter, std::vector<u16> allowedUnitIds)
{

}

void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func)
{

}

Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func)
{
	return nullptr;
}

Unit* unit_array_get_first(UnitArray* unit_array, SearchFilterFunc match_func)
{
	if (!match_func)
	{
		if (unit_array->count)
		{
			return unit_array->units[0];
		}
		return nullptr;
	}

	for (u32 i = 0; i < unit_array->count; i++)
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
			array->count = 0;
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

bool unit_flag_match(Unit* center, Unit* unit, u32 context, std::vector<u16> allowedUnitIds)
{
	if (!(context & SearchFlagsIncludingSelf) && unit == center)
	{
		return false;
	}
	if (context & SearchFlagsNotSelf && unit == center)
	{
		return false;
	}
	if (!(context & SearchFlagsInvincible) && unit->status & UnitStatus::Invincible)
	{
		return false;
	}
	if (context & SearchFlagsNoAir && unit->status & UnitStatus::InAir)
	{
		return false;
	}
	if (context & SearchFlagsNoGround && !(unit->status & UnitStatus::InAir))
	{
		return false;
	}
	if (context & SearchFlagsSpellcaster)
	{
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
		{
			return false;
		}
	}
	if (context & SearchFlagsHasEnergy)
	{
		if (unit->energy < 256)
		{
			return false;
		}
	}
	if (context & SearchFlagsShielded)
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
	if (context & SearchFlagsPowered)
	{
		if (unit->status & UnitStatus::DoodadStatesThing)
		{
			return false;
		}
	}
	if (context & SearchFlagsCompleted)
	{
		if (!(unit->status & UnitStatus::Completed))
		{
			return false;
		}
	}
	if (context & SearchFlagsNoTrap)
	{
		if (scbw::unitIsTrap(unit))
		{
			return false;
		}
	}
	if (context & SearchFlagsNoHallucination)
	{
		if (unit->status & UnitStatus::IsHallucination)
		{
			return false;
		}
	}
	if (center)
	{
		if (context & SearchFlagsFrontalArc)
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
		if (context & SearchFlagsOwn && unit->playerId != center->playerId)
		{
			return false;
		}
		if (scbw::isAlliedTo(unit->playerId, center->playerId) && context & SearchFlagsEnemy)
		{
			return false;
		}
		if (!scbw::isAlliedTo(unit->playerId, center->playerId) && context & SearchFlagsAlly)
		{
			return false;
		}
	}
	if (allowedUnitIds.size() > 0)
	{
		bool good = false;
		for (auto a : allowedUnitIds)
		{
			if (unit->id == a)
			{
				good = true;
				break;
			}
		}
		if (!good)
		{
			return false;
		}
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && context & SearchFlagsNoOrganic)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && context & SearchFlagsOnlyOrganic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && context & SearchFlagsNoMechanical)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && context & SearchFlagsOnlyMechanic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && context & SearchFlagsNoBuilding)
	{
		return false;
	}
	if (!(scbw::isEnabled(unit)) && context & SearchFlagsEnabled)
	{
		return false;
	}
	if (context & SearchFlagsCanAttackOrCast && !unit->hasWeapon() && !(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
	{
		return false;
	}
	if (context & SearchFlagsNotWorker && units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
	{
		return false;
	}
	return true;
}
