#pragma once
#include <BW\bwdata.h>
#include <globals.h>
#include <functional>
#include <unordered_set>

enum SearchFilter
{
	FilterInvincible			= 1 <<  0,
	FilterNoAir					= 1 <<  1,
	FilterNoGround				= 1 <<  2,
	FilterNoMechanical			= 1 <<  3,
	FilterNoOrganic				= 1 <<  4,
	FilterNoBuilding			= 1 <<  5,
	FilterOnlyOrganic			= 1 <<  6,
	FilterOnlyMechanic			= 1 <<  7,
	FilterEnemy					= 1 <<  8,
	FilterAlly					= 1 <<  9,
	FilterIncludingSelf			= 1 << 10,
	FilterFrontalArc			= 1 << 11,
	FilterSpellcaster			= 1 << 12,
	FilterHasEnergy				= 1 << 13,
	FilterNotSelf				= 1 << 14,
	FilterShielded				= 1 << 15,
	FilterEnabled				= 1 << 16,
	FilterPowered				= 1 << 17,
	FilterCompleted				= 1 << 18,
	FilterNoTrap				= 1 << 19,
	FilterNoHallucination		= 1 << 20,
	FilterCanAttackOrCast		= 1 << 21,
	FilterOwn					= 1 << 22,
	FilterNotWorker				= 1 << 23,
	FilterNotSubunit			= 1 << 24,
	FilterBuilding				= 1 << 25,
	FilterNoEgg					= 1 << 26,
	FilterNoLarva				= 1 << 27,
	FilterNoHangar				= 1 << 28,
	FilterVisible				= 1 << 29,
};

enum SearchFilterCorpse
{
	CorpseFilterNoAir			= 1 <<  1,
	CorpseFilterNoGround		= 1 <<  2,
	CorpseFilterNoBuilding		= 1 <<  3,
	CorpseFilterEnemy			= 1 <<  4,
	CorpseFilterAlly			= 1 <<  5,
	CorpseFilterIncludingSelf	= 1 <<  6,
	CorpseFilterSpellcaster		= 1 <<  7,
	CorpseFilterNotSelf			= 1 <<  8,
	CorpseFilterShielded		= 1 <<  9,
	CorpseFilterNoTrap			= 1 << 10,
	CorpseFilterOwn				= 1 << 11,
	CorpseFilterNotWorker		= 1 << 12,
	CorpseFilterNotSubunit		= 1 << 13,
	CorpseFilterBuilding		= 1 << 14,
	CorpseFilterNoEgg			= 1 << 15,
	CorpseFilterNoLarva			= 1 << 16,
	CorpseFilterNoHangar		= 1 << 17,
	CorpseFilterVisible			= 1 << 18,
};

#define ScoreInvalidUnit INT_MIN
#define EntryLimit UnitLimit

template <typename T>
struct EntryArrayInternal {
	u32 count;
	T data[EntryLimit];

	T& operator[](u32 index) {
		return data[index];
	}

	T* begin() {
		return data;
	}

	T* end() {
		return data + count;
	}
};

union EntryArray {
	u32 count;
	EntryArrayInternal<void*> entries;
	EntryArrayInternal<Unit*> units;
	EntryArrayInternal<Bullet*> bullets;
	EntryArrayInternal<Thingy*> corpses;
	EntryArrayInternal<EffectField*> effects;
};

void entry_array_add(EntryArray* array, void* data);
void entry_array_remove(EntryArray* array, u32 index); // won't preserve order

// NOTE:
// not a fan of using an STL container, but we already have that dependency in the code base
// also not sure how I would handle variadic argument like that, without refactoring everything that exists in this project
// optional allowance of lambdas is pretty handy, since most of behaviors need adhoc adjustments
using SearchFilterFunc = std::function<bool(Unit*)>;
using SearchScoreFunc = std::function<s32(Unit*)>;

void unit_search_rect(EntryArray* entry_array, Box32 rect);
void unit_search_square(EntryArray* entry_array, Point16 center, u32 radius);
void unit_search_radial(EntryArray* entry_array, Point16 center, u32 radius);

void seen_unit_search_rect(EntryArray* entry_array, u32 player, Box32 rect);
void seen_unit_search_square(EntryArray* entry_array, u32 player, Point16 center, u32 radius);
void seen_unit_search_radial(EntryArray* entry_array, u32 player, Point16 center, u32 radius);

void bullet_search_rect(EntryArray* entry_array, Box32 rect);
void bullet_search_square(EntryArray* entry_array, Point16 center, u32 radius);
void bullet_search_radial(EntryArray* entry_array, Point16 center, u32 radius);

void corpse_search_rect(EntryArray* entry_array, Box32 rect);
void corpse_search_square(EntryArray* entry_array, Point16 center, u32 radius);
void corpse_search_radial(EntryArray* entry_array, Point16 center, u32 radius);

void effect_search_rect(EntryArray* entry_array, Box32 rect);
void effect_search_square(EntryArray* entry_array, Point16 center, u32 radius);
void effect_search_radial(EntryArray* entry_array, Point16 center, u32 radius);

// TODO: combine filter and search together for ease of use (unit_search_radial_ex)
void unit_search_filter(EntryArray* unit_array, Unit* reference, u32 flags = 0, std::unordered_set<u16> allowed_unit_ids = {}); // will break ordering, do sorting after filtering!
void corpse_search_filter(EntryArray* corpse_array, Thingy* reference, u32 flags, std::unordered_set<u16> allowed_unit_ids);
void unit_search_custom_filter(EntryArray* unit_array, SearchFilterFunc filter); // will break ordering, prefer sorting after filter
void unit_search_sort(EntryArray* unit_array, SearchScoreFunc score_func);
void unit_search_sort_by_distance(EntryArray* unit_array, Point16 center);
void corpse_search_sort_by_distance(EntryArray* unit_array, Point16 center);
Unit* unit_search_find_best(EntryArray* unit_array, SearchScoreFunc score_func = nullptr); // score_func defaults to distance check
Unit* unit_search_get_first(EntryArray* unit_array, SearchFilterFunc match_func = nullptr); // defaults to no check
bool unit_flag_match(Unit* unit, Unit* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids = {});
bool corpse_flag_match(Thingy* target, Thingy* center, u32 flags, const std::unordered_set<u16>& allowed_unit_ids);
void unit_search_add_units_from_connected_transport(EntryArray* unit_array, Unit* unit);

void reset_search_index();
void update_search_index(); // must be done once per frame
void update_search_index_effects(); // must be done once per frame after effects are updated

bool is_point_in_rect(Point16 point, Box32 rect);
bool is_rect_in_rect(Box32 inner, Box32 outer);