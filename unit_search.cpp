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

static bool is_in_rect(Point16 point, Box32 box)
{
	return true
		&& point.x >= box.left
		&& point.x <= box.right
		&& point.y >= box.top
		&& point.y <= box.bottom;
}

UnitArray search_rect(Box32 rect)
{
	UnitArray array;
	array.count = 0;

	u32 x1 = rect.left / CELL_SIZE;
	u32 x2 = ceil(rect.right / (f32)CELL_SIZE);
	u32 y1 = rect.top / CELL_SIZE;
	u32 y2 = ceil(rect.bottom / (f32)CELL_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is entirely covered by rect
			u32 tile_x1 = rect.left / CELL_SIZE;
			u32 tile_x2 = rect.right / CELL_SIZE + 1;
			u32 tile_y1 = rect.top / CELL_SIZE;
			u32 tile_y2 = rect.bottom / CELL_SIZE + 1;

			bool tileIsFullyCovered = false;

			// check if unit is in the rect
			UnitArray* array = &unit_grid[x][y];
			for (u32 i = 0; i < array->count; i++)
			{
				Unit* unit = array->units[i];
				
				if (!tileIsFullyCovered && !is_in_rect(unit->position, rect))
				{
					continue;
				}

				unit_array_add(array, unit);
			}
		}
	}

	return array;
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
	UnitArray array;
	array.count = 0;

	return array;
}

void unit_array_filter(UnitArray* unit_array, u32 flags, SearchFilterFunc filter, std::vector<u16> allowedUnitIds)
{

}

void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func)
{

}

void unit_array_sort_by_distance(UnitArray* unit_array, Point16 center)
{

}

Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func)
{
	return nullptr;
}

static void reset_unit_grid()
{
	for (u32 x = 0; x < CELL_COUNT; x++)
	{
		for (u32 y = 0; y < CELL_COUNT; y++)
		{
			UnitArray* array = &unit_grid[x][y];
			array->count = 0;
		}
	}
}

void update_unit_grid()
{
	reset_unit_grid();
	for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
	{
		u32 x = unit->position.x / 32 / CELL_SIZE;
		u32 y = unit->position.y / 32 / CELL_SIZE;
		UnitArray* array = &unit_grid[x][y];
		unit_array_add(array, unit);
	}
}

static bool unit_flag_match(Unit* center, Unit* unit, u32 context, std::vector<u16> allowedUnitIds)
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
