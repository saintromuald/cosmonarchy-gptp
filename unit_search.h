#pragma once
#include <SCBW\scbwdata.h>
#include <functional>
#include <unordered_set>

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
	u32 size;
	Unit* units[1700];
};

inline void unit_array_add(UnitArray* array, Unit* unit);
inline void unit_array_remove_fast(UnitArray* array, u32 index); // won't preserve order
void unit_array_remove(UnitArray* array, u32 index); // will preserve order
void unit_array_combine(UnitArray* target, UnitArray* source);

// NOTES:
// not a fan of using an STL container, but we already have that dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
using SearchScoreFunc = std::function<u32(Unit*)>;
UnitArray search_rect(Box32 rect);
UnitArray search_radial(Point16 center, u32 radius);
inline UnitArray search_square(Point16 center, u32 radius);

void unit_array_filter(UnitArray* unit_array, Unit* center, u32 flags = 0, std::unordered_set<u16> allowedUnitIds = {}); // will break ordering, prefer sorting after filter
void unit_array_custom_filter(UnitArray* unit_array, SearchFilterFunc filter); // will break ordering, prefer sorting after filter
void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func);
void unit_array_sort_by_distance(UnitArray* unit_array, Point16 center);
Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func = nullptr); // score_func defaults to distance check
Unit* unit_array_get_first(UnitArray* unit_array, SearchFilterFunc match_func = nullptr); // defaults to no check
bool unit_flag_match(Unit* unit, Unit* center, u32 context, std::unordered_set<u16> allowedUnitIds);

void update_unit_grid(); // must be done once per frame

inline bool is_point_in_rect(Point16 point, Box32 rect);
inline bool is_rect_in_rect(Box32 inner, Box32 outer);