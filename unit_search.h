#pragma once
#include <SCBW\scbwdata.h>
#include <functional>
#include <unordered_set>

// GENERAL NOTES:
// more performant version of unit finder, with more reasonable syntax

enum SearchFilter
{
	FilterInvincible		= 1 <<  0,
	FilterNoAir			= 1 <<  1,
	FilterNoGround			= 1 <<  2,
	FilterNoMechanical		= 1 <<  3,
	FilterNoOrganic		= 1 <<  4,
	FilterNoBuilding		= 1 <<  5,
	FilterOnlyOrganic		= 1 <<  6,
	FilterOnlyMechanic		= 1 <<  7,
	FilterEnemy			= 1 <<  8,
	FilterAlly				= 1 <<  9,
	FilterIncludingSelf	= 1 << 10,
	FilterFrontalArc		= 1 << 11,
	FilterSpellcaster		= 1 << 12,
	FilterHasEnergy		= 1 << 13,
	FilterNotSelf			= 1 << 14,
	FilterShielded			= 1 << 15,
	FilterEnabled			= 1 << 16,
	FilterPowered			= 1 << 17,
	FilterCompleted		= 1 << 18,
	FilterNoTrap			= 1 << 19,
	FilterNoHallucination	= 1 << 20,
	FilterCanAttackOrCast	= 1 << 21,
	FilterOwn				= 1 << 22,
	FilterNotWorker		= 1 << 23,
};

struct UnitArray
{
	u32 size;
	Unit* units[1700];
};

inline void unit_array_add(UnitArray* array, Unit* unit);
inline void unit_array_remove_fast(UnitArray* array, u32 index); // won't preserve order, will swap with last element
void unit_array_remove(UnitArray* array, u32 index); // will preserve order

// NOTES:
// not a fan of using an STL container, but we already have that dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
using SearchScoreFunc = std::function<u32(Unit*)>;
UnitArray unit_search_rect(Box32 rect);
UnitArray unit_search_radial(Point16 center, u32 radius);
inline UnitArray unit_search_square(Point16 center, u32 radius);

void unit_array_filter(UnitArray* unit_array, Unit* reference, u32 flags = 0, std::unordered_set<u16> allowed_unit_ids = {}); // will break ordering, prefer sorting after filter
void unit_array_custom_filter(UnitArray* unit_array, SearchFilterFunc filter); // will break ordering, prefer sorting after filter
void unit_array_sort(UnitArray* unit_array, SearchScoreFunc score_func);
void unit_array_sort_by_distance(UnitArray* unit_array, Point16 center);
Unit* unit_array_find_best(UnitArray* unit_array, SearchScoreFunc score_func = nullptr); // score_func defaults to distance check
Unit* unit_array_get_first(UnitArray* unit_array, SearchFilterFunc match_func = nullptr); // defaults to no check
bool unit_flag_match(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids);

void update_unit_grid(); // must be done once per frame

inline bool is_point_in_rect(Point16 point, Box32 rect);
inline bool is_rect_in_rect(Box32 inner, Box32 outer);