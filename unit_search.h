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

struct UnitArray
{
	u32 size;
	Unit* units[1700];
};

inline UnitArray UnitArrayDuplicate(UnitArray* array);
inline void UnitArrayAdd(UnitArray* array, Unit* unit);
inline void UnitArrayRemoveFast(UnitArray* array, u32 index); // won't preserve order, will swap with last element
inline void UnitArrayRemove(UnitArray* array, u32 index); // will preserve order

// NOTES:
// not a fan of using an STL container, but we already have that dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
using SearchScoreFunc = std::function<u32(Unit*)>;
UnitArray UnitSearchRect(Box32 rect);
UnitArray UnitSearchRadial(Point16 center, u32 radius);
inline UnitArray UnitSearchSquare(Point16 center, u32 radius);

void UnitArrayFilter(UnitArray* unit_array, Unit* reference, u32 flags = 0, std::unordered_set<u16> allowed_unit_ids = {}); // will break ordering, prefer sorting after filter
void UnitArrayCustomFilter(UnitArray* unit_array, SearchFilterFunc filter); // will break ordering, prefer sorting after filter
void UnitArraySort(UnitArray* unit_array, SearchScoreFunc score_func);
void UnitArraySortByDistance(UnitArray* unit_array, Point16 center);
Unit* UnitArrayFindBest(UnitArray* unit_array, SearchScoreFunc score_func = nullptr); // score_func defaults to distance check
Unit* UnitArrayGetFirst(UnitArray* unit_array, SearchFilterFunc match_func = nullptr); // defaults to no check
bool UnitFlagMatch(Unit* unit, Unit* center, u32 flags, std::unordered_set<u16> allowed_unit_ids);

void UpdateUnitGrid(); // must be done once per frame

inline bool IsPointInRect(Point16 point, Box32 rect);
inline bool IsRectInRect(Box32 inner, Box32 outer);