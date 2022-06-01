#pragma once
#include <SCBW\scbwdata.h>
#include <functional>
#include <unordered_set>

// GENERAL NOTES:
// more performant version of unit finder, with more reasonable syntax

enum SearchFilter
{
	FilterInvincible		= 1 <<  0,
	FilterNoAir				= 1 <<  1,
	FilterNoGround			= 1 <<  2,
	FilterNoMechanical		= 1 <<  3,
	FilterNoOrganic			= 1 <<  4,
	FilterNoBuilding		= 1 <<  5,
	FilterOnlyOrganic		= 1 <<  6,
	FilterOnlyMechanic		= 1 <<  7,
	FilterEnemy				= 1 <<  8,
	FilterAlly				= 1 <<  9,
	FilterIncludingSelf		= 1 << 10,
	FilterFrontalArc		= 1 << 11,
	FilterSpellcaster		= 1 << 12,
	FilterHasEnergy			= 1 << 13,
	FilterNotSelf			= 1 << 14,
	FilterShielded			= 1 << 15,
	FilterEnabled			= 1 << 16,
	FilterPowered			= 1 << 17,
	FilterCompleted			= 1 << 18,
	FilterNoTrap			= 1 << 19,
	FilterNoHallucination	= 1 << 20,
	FilterCanAttackOrCast	= 1 << 21,
	FilterOwn				= 1 << 22,
	FilterNotWorker			= 1 << 23,
	FilterBuilding			= 1 << 25,
};

#define ScoreInvalidUnit INT_MIN

struct UnitArray
{
	u32 size;
	Unit* units[1700];
};

struct CorpseArray {
	u32 size;
	Thingy* corpses[1700];
};

void unit_array_add(UnitArray* array, Unit* unit);
void unit_array_remove_fast(UnitArray* array, u32 index); // won't preserve order, will swap with last element
void unit_array_remove(UnitArray* array, u32 index); // will preserve order

void corpse_array_add(CorpseArray* array, Thingy* unit);
void corpse_array_remove_fast(CorpseArray* array, u32 index); // won't preserve order, will swap with last element
void corpse_array_remove(CorpseArray* array, u32 index); // will preserve order

// NOTES:
// not a fan of using an STL container, but we already have that dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
using SearchScoreFunc = std::function<s32(Unit*)>;
UnitArray unit_search_rect(Box32 rect);
UnitArray unit_search_square(Point16 center, u32 radius);
UnitArray unit_search_radial(Point16 center, u32 radius);

CorpseArray corpse_search_rect(Box32 rect);
CorpseArray corpse_search_square(Point16 center, u32 radius);
CorpseArray corpse_search_radial(Point16 center, u32 radius);

// TODO: combine filter and search together for ease of use (unit_search_radial_ex)
void unit_search_filter(UnitArray* unit_array, Unit* reference, u32 flags = 0, std::unordered_set<u16> allowed_unit_ids = {}); // will break ordering, prefer sorting after filter
void unit_search_custom_filter(UnitArray* unit_array, SearchFilterFunc filter); // will break ordering, prefer sorting after filter
void unit_search_sort(UnitArray* unit_array, SearchScoreFunc score_func);
void unit_search_sort_by_distance(UnitArray* unit_array, Point16 center);
Unit* unit_search_find_best(UnitArray* unit_array, SearchScoreFunc score_func = nullptr); // score_func defaults to distance check
Unit* unit_search_get_first(UnitArray* unit_array, SearchFilterFunc match_func = nullptr); // defaults to no check
bool unit_flag_match(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids);

void update_search_grid(); // must be done once per frame

bool is_point_in_rect(Point16 point, Box32 rect);
bool is_rect_in_rect(Box32 inner, Box32 outer);