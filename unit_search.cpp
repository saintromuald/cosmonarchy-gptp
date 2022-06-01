#include "unit_search.h"
#include <SCBW\api.h>

#define CELL_SIZE 8
#define CELL_PX_SIZE (CELL_SIZE * 32)
#define CELL_COUNT (256 / CELL_SIZE)

// 8x8 tile cells for up to 256 map size
// NOTE: static will init grid to zeros, and limit scope to this file
// well not really, because unity build, but w/e
static UnitArray unit_grid[CELL_COUNT][CELL_COUNT];
static CorpseArray corpse_grid[CELL_COUNT][CELL_COUNT];

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

// BINARY SEARCH BASED (unit finder had it, badly implemented though):
// - has to sort whole unit array by X and Y
// - always results in O(n) of distance checks
// - still better than old unit finder, since it would sort only once per frame,
// not every finder proc
// - not sure how imperformant array intersection will be

inline void UnitArrayAdd(UnitArray* array, Unit* unit)
{
	array->units[array->size++] = unit;
}

inline void UnitArrayRemoveFast(UnitArray* array, u32 index)
{
	array->units[index] = array->units[--array->size];
}

inline void UnitArrayRemove(UnitArray* array, u32 index)
{
	for (u32 i = array->size - 1; i >= index; i--)
	{
		array->units[i] = array->units[i + 1];
	}
}

inline void CorpseArrayAdd(CorpseArray* array, Thingy* corpse)
{
	array->corpses[array->size++] = corpse;
}

inline void CorpseArrayRemoveFast(CorpseArray* array, u32 index)
{
	array->corpses[index] = array->corpses[--array->size];
}

inline void CorpseArrayRemove(CorpseArray* array, u32 index)
{
	for (u32 i = array->size - 1; i >= index; i--)
	{
		array->corpses[i] = array->corpses[i + 1];
	}
}

bool IsPointInRect(Point16 point, Box32 rect)
{
	return true
		&& point.x >= rect.left
		&& point.x <= rect.right
		&& point.y >= rect.top
		&& point.y <= rect.bottom;
}

bool IsRectInRect(Box32 inner, Box32 outer)
{
	return true
		&& outer.left <= inner.left
		&& inner.right <= outer.right
		&& outer.top <= inner.top
		&& inner.bottom <= outer.bottom;
}

UnitArray UnitSearchRect(Box32 rect)
{
	rect = scbw::clampRect(rect);
	UnitArray unit_array = {};

	const u32 x1 = rect.left / CELL_PX_SIZE;
	const u32 y1 = rect.top / CELL_PX_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(rect.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is worth evaluating
			UnitArray* current_array = &unit_grid[x][y];
			if (current_array->size)
			{
				// check if tile is entirely covered by the area
				const Box32 tile_rect =
				{
					x * CELL_PX_SIZE,
					y * CELL_PX_SIZE,
					x * CELL_PX_SIZE + CELL_PX_SIZE,
					y * CELL_PX_SIZE + CELL_PX_SIZE
				};
				const bool tileIsFullyCovered = IsRectInRect(tile_rect, rect);

				// check which units are in the area
				for (u32 i = 0; i < current_array->size; i++)
				{
					Unit* unit = current_array->units[i];
					if (unit->isDead()) {
						continue;
					}
					if (!tileIsFullyCovered && !IsPointInRect(unit->position, rect))
					{
						continue;
					}
					UnitArrayAdd(&unit_array, unit);
				}
			}
		}
	}

	return unit_array;
}

inline UnitArray UnitSearchSquare(Point16 center, u32 radius)
{
	return UnitSearchRect({
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	});
}

UnitArray unit_search_radial(Point16 center, u32 radius)
{
	UnitArray unit_array = {};

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
			// check if tile is worth evaluating
			UnitArray* current_array = &unit_grid[x][y];
			if (current_array->size)
			{
				// check if tile is entirely covered by the area
				const Point16 tile_center =
				{
					x * CELL_PX_SIZE + CELL_PX_SIZE / 2,
					y * CELL_PX_SIZE + CELL_PX_SIZE / 2
				};
				const bool tileIsFullyCovered = scbw::get_distance_fast(center, tile_center) + CELL_PX_SIZE * 1.414 <= radius;

				// check which units are in the area
				for (u32 i = 0; i < current_array->size; i++)
				{
					Unit* unit = current_array->units[i];
					if (unit->isDead()) {
						continue;
					}
					if (!tileIsFullyCovered && scbw::get_distance_fast(center, unit->position) > radius)
					{
						continue;
					}
					UnitArrayAdd(&unit_array, unit);
				}
			}
		}
	}

	return unit_array;
}

CorpseArray CorpseSearchRect(Box32 rect) {
	rect = scbw::clampRect(rect);
	CorpseArray corpse_array = {};

	const u32 x1 = rect.left / CELL_PX_SIZE;
	const u32 y1 = rect.top / CELL_PX_SIZE;
	const u32 x2 = ceil(rect.right / (f32)CELL_PX_SIZE);
	const u32 y2 = ceil(rect.bottom / (f32)CELL_PX_SIZE);

	for (u32 x = x1; x < x2; x++)
	{
		for (u32 y = y1; y < y2; y++)
		{
			// check if tile is worth evaluating
			CorpseArray* current_array = &corpse_grid[x][y];
			if (current_array->size)
			{
				// check if tile is entirely covered by the area
				const Box32 tile_rect =
				{
					x * CELL_PX_SIZE,
					y * CELL_PX_SIZE,
					x * CELL_PX_SIZE + CELL_PX_SIZE,
					y * CELL_PX_SIZE + CELL_PX_SIZE
				};
				const bool tileIsFullyCovered = IsRectInRect(tile_rect, rect);

				// check which units are in the area
				for (u32 i = 0; i < current_array->size; i++)
				{
					Thingy* corpse = current_array->corpses[i];
					if (corpse->sprite->flags & SpriteFlags::Flag04) {
						continue;
					}
					if (!tileIsFullyCovered && !IsPointInRect(corpse->sprite->position, rect))
					{
						continue;
					}
					CorpseArrayAdd(&corpse_array, corpse);
				}
			}
		}
	}

	return corpse_array;
}

inline CorpseArray CorpseSearchSquare(Point16 center, u32 radius) {
	return CorpseSearchRect({
		center.x - (s32)radius,
		center.y - (s32)radius,
		center.x + (s32)radius,
		center.y + (s32)radius
	});
}

CorpseArray corpse_search_radial(Point16 center, u32 radius) {
	CorpseArray corpse_array = {};

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
			// check if tile is worth evaluating
			CorpseArray* current_array = &corpse_grid[x][y];
			if (current_array->size)
			{
				// check if tile is entirely covered by the area
				const Point16 tile_center =
				{
					x * CELL_PX_SIZE + CELL_PX_SIZE / 2,
					y * CELL_PX_SIZE + CELL_PX_SIZE / 2
				};
				const bool tileIsFullyCovered = scbw::get_distance_fast(center, tile_center) + CELL_PX_SIZE * 1.414 <= radius;

				// check which units are in the area
				for (u32 i = 0; i < current_array->size; i++)
				{
					Thingy* corpse = current_array->corpses[i];
					if (corpse->sprite->flags & SpriteFlags::Flag04) {
						continue;
					}
					if (!tileIsFullyCovered && scbw::get_distance_fast(center, corpse->sprite->position) > radius)
					{
						continue;
					}
					CorpseArrayAdd(&corpse_array, corpse);
				}
			}
		}
	}

	return corpse_array;
}

void UnitArrayFilter(UnitArray* unit_array, Unit* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids)
{
	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		if (!UnitFlagMatch(unit, reference, flags, allowed_unit_ids))
		{
			UnitArrayRemoveFast(unit_array, i--);
			continue;
		}
	}
}

void UnitArrayCustomFilter(UnitArray* unit_array, SearchFilterFunc filter)
{
	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		if (!filter(unit))
		{
			UnitArrayRemoveFast(unit_array, i--);
			continue;
		}
	}
}

void UnitArraySort(UnitArray* unit_array, SearchScoreFunc score_func)
{
	// not a fan of std::sort, probably should replace with own qsort
	// qsort from C can't work with lambdas
	std::sort(unit_array->units, unit_array->units + unit_array->size, [&](Unit* a, Unit* b)
	{
		return score_func(a) > score_func(b);	
	});
}

static Point16 TempCenter;
static s32 DistanceCompareFunc(void const* a, void const* b)
{
	return scbw::get_distance_fast(TempCenter, ((Unit*)a)->position) > scbw::get_distance_fast(TempCenter, ((Unit*)b)->position);	
}

void UnitArraySortByDistance(UnitArray* unit_array, Point16 center)
{
	TempCenter = center;
	qsort(unit_array->units, unit_array->size, sizeof(Unit*), DistanceCompareFunc);
}

Unit* UnitArrayFindBest(UnitArray* unit_array, SearchScoreFunc score_func)
{
	s32 best_score = ScoreInvalidUnit;
	Unit* best_unit = 0;

	for (u32 i = 0; i < unit_array->size; i++)
	{
		Unit* unit = unit_array->units[i];
		s32 score = score_func(unit);
		if (score > best_score)
		{
			best_unit = unit;
			best_score = score;
		}
	}

	return best_unit;
}

Unit* UnitArrayGetFirst(UnitArray* unit_array, SearchFilterFunc match_func)
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

void update_search_grid()
{
	// Reset units
	for (u32 x = 0; x < CELL_COUNT; x++)
	{
		for (u32 y = 0; y < CELL_COUNT; y++)
		{
			UnitArray* array = &unit_grid[x][y];
			array->size = 0;
		}
	}

	// Update units
	for (Unit* unit = *firstVisibleUnit; unit; unit = unit->link.next)
	{
		u32 x = unit->position.x / 32 / CELL_SIZE;
		u32 y = unit->position.y / 32 / CELL_SIZE;
		UnitArray* array = &unit_grid[x][y];
		UnitArrayAdd(array, unit);
	}

	// Reset corpses
	for (u32 x = 0; x < CELL_COUNT; x++)
	{
		for (u32 y = 0; y < CELL_COUNT; y++)
		{
			CorpseArray* array = &corpse_grid[x][y];
			array->size = 0;
		}
	}

	// Update corpses
	for (Thingy* thingy = *first_lone_sprite; thingy; thingy = thingy->next)
	{
		if (thingy->sprite)
		{
			switch (thingy->sprite->spriteId)
			{
				case SpriteId::Corpse_Apostle:
				case SpriteId::Corpse_Aspirant:
				case SpriteId::Corpse_Bactalisk:
				case SpriteId::Corpse_Blackjack:
				case SpriteId::Corpse_Broodling:
				case SpriteId::Corpse_Cleric:
				case SpriteId::Corpse_Dracadin:
				case SpriteId::Corpse_Droleth:
				case SpriteId::Corpse_Egg:
				case SpriteId::Corpse_Eidolon:
				case SpriteId::Corpse_Harakan:
				case SpriteId::Corpse_Hydralisk:
				case SpriteId::Corpse_Isthrathaleth:
				case SpriteId::Corpse_Izirokor:
				case SpriteId::Corpse_Konvilisk:
				case SpriteId::Corpse_Lakizilisk:
				case SpriteId::Corpse_Larva:
				case SpriteId::Corpse_Maverick:
				case SpriteId::Corpse_Scribe:
				case SpriteId::Corpse_Ultrakor:
				case SpriteId::Corpse_Zergling:
				case SpriteId::Corpse_TerranBuildingLarge:
				case SpriteId::Corpse_TerranBuildingSmall:
				case SpriteId::Corpse_ProtossBuildingLarge:
				case SpriteId::Corpse_ProtossBuildingSmall:
				case SpriteId::Corpse_ZergBuildingLarge:
				case SpriteId::Corpse_ZergBuildingSmall:
				{
					u32 x = thingy->sprite->position.x / 32 / CELL_SIZE;
					u32 y = thingy->sprite->position.y / 32 / CELL_SIZE;
					CorpseArray* array = &corpse_grid[x][y];
					CorpseArrayAdd(array, thingy);
				} break;
			}
		}
	}
}

bool UnitFlagMatch(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids)
{
	if (!(flags & FilterIncludingSelf) && unit == center)
	{
		return false;
	}
	if (flags & FilterNotSelf && unit == center)
	{
		return false;
	}
	if (!(flags & FilterInvincible) && unit->status & UnitStatus::Invincible)
	{
		return false;
	}
	if (flags & FilterNoAir && unit->status & UnitStatus::InAir)
	{
		return false;
	}
	if (flags & FilterNoGround && !(unit->status & UnitStatus::InAir))
	{
		return false;
	}
	if (flags & FilterSpellcaster)
	{
		if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
		{
			return false;
		}
	}
	if (flags & FilterHasEnergy)
	{
		if (unit->energy < 256)
		{
			return false;
		}
	}
	if (flags & FilterShielded)
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
	if (flags & FilterPowered)
	{
		if (unit->status & UnitStatus::Frozen)
		{
			return false;
		}
	}
	if (flags & FilterCompleted)
	{
		if (!(unit->status & UnitStatus::Completed))
		{
			return false;
		}
	}
	if (flags & FilterNoTrap)
	{
		if (scbw::unit_is_trap(unit))
		{
			return false;
		}
	}
	if (flags & FilterNoHallucination)
	{
		if (unit->status & UnitStatus::IsHallucination)
		{
			return false;
		}
	}
	if (center)
	{
		if (flags & FilterFrontalArc)
		{
			u16 angle = scbw::get_angle(center->position.x, center->position.y,
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
		if (flags & FilterOwn && unit->player != center->player)
		{
			return false;
		}
		if (scbw::is_allied_to(unit->player, center->player) && flags & FilterEnemy)
		{
			return false;
		}
		if (!scbw::is_allied_to(unit->player, center->player) && flags & FilterAlly)
		{
			return false;
		}
	}
	if (allowed_unit_ids.size() && allowed_unit_ids.find(unit->id) == allowed_unit_ids.end())
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Organic && flags & FilterNoOrganic)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Organic) && flags & FilterOnlyOrganic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical && flags & FilterNoMechanical)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Mechanical) && flags & FilterOnlyMechanic)
	{
		return false;
	}
	if (units_dat::BaseProperty[unit->id] & UnitProperty::Building && flags & FilterNoBuilding)
	{
		return false;
	}
	if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building) && flags & FilterBuilding)
	{
		return false;
	}
	if (!(scbw::isEnabled(unit)) && flags & FilterEnabled)
	{
		return false;
	}
	if (flags & FilterCanAttackOrCast && !unit->hasWeapon() && !(units_dat::BaseProperty[unit->id] & UnitProperty::Spellcaster))
	{
		return false;
	}
	if (flags & FilterNotWorker && units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
	{
		return false;
	}
	return true;
}
