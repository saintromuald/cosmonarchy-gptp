#pragma once
#include <SCBW\scbwdata.h>
#include <functional>

// GENERAL NOTES:
// more performant version of unit finder, with more reasonable syntax

enum SearchFlags
{
	SearchFlagsInvincible		= 1 <<  0,
	SearchFlagsNoAir			= 1 <<  1,
	SearchFlagsNoGround			= 1 <<  2,
	SearchFlagsNoMechanical		= 1 <<  3,
	SearchFlagsNoOrganic		= 1 <<  4,
	SearchFlagsNoBuilding		= 1 <<  5,
	SearchFlagsOnlyOrganic		= 1 <<  6,
	SearchFlagsOnlyMechanic		= 1 <<  7,
	SearchFlagsEnemy			= 1 <<  8,
	SearchFlagsAlly				= 1 <<  9,
	SearchFlagsIncludingSelf	= 1 << 10,
	SearchFlagsFrontalArc		= 1 << 11,
	SearchFlagsSpellcaster		= 1 << 12,
	SearchFlagsHasEnergy		= 1 << 13,
	SearchFlagsNotSelf			= 1 << 14,
	SearchFlagsShielded			= 1 << 15,
	SearchFlagsEnabled			= 1 << 16,
	SearchFlagsPowered			= 1 << 17,
	SearchFlagsCompleted		= 1 << 18,
	SearchFlagsNoTrap			= 1 << 19,
	SearchFlagsNoHallucination	= 1 << 20,
	SearchFlagsCanAttackOrCast	= 1 << 21,
	SearchFlagsOwn				= 1 << 22,
	SearchFlagsNotWorker		= 1 << 23,
};

struct UnitArray
{
	u32 count;
	Unit* units[1700];
};

inline void unit_array_add(UnitArray* array, Unit* unit);
inline void unit_array_remove_fast(UnitArray* array, u32 index); // won't preserve order
void unit_array_remove(UnitArray* array, u32 index); // will preserve order
void unit_array_combine(UnitArray* target, UnitArray* source);

// NOTES:
// not a fan of using a vector, but we already have the dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything
// that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
UnitArray search_rect(Box32 rect);
UnitArray search_radial(Point16 center, u32 radius);
inline UnitArray search_square(Point16 center, u32 radius);

using SearchScoreFunc = std::function<u32(Unit*)>;
void unit_array_filter(UnitArray* unit_array, u32 flags = 0, SearchFilterFunc filter = nullptr, std::vector<u16> allowedUnitIds = {});
void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func);
void unit_array_sort_by_distance(UnitArray* unit_array, Point16 center);
Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func);
bool unit_flag_match(Unit* center, Unit* unit, u32 context, std::vector<u16> allowedUnitIds);

void update_unit_grid(); // must be done once per frame

bool is_in_rect(Point16 point, Box16 box);