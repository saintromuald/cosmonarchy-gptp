#pragma once
#include "../BW/UnitFinder.h"
#pragma once
#include "../BW/structures/Unit.h"
#include "../BW/api.h"

namespace AI
{
	bool is_target_worth_hitting(Unit* target, Unit* attacker);

	// Returns true if @p target is attacking a player allied to @p unit.
	bool is_target_attacking_ally(Unit* target, Unit* unit);

	// Determines whether the given @p player is in a UMS map (i.e. campaign mode).
	bool is_campaign_ai(u8 player);

	// Determines if the @p unit is within a region that is not commanded by its controlling AI. Details are not really understood.
	bool unit_is_in_unsafe_region(Unit* unit);

	s32 get_total_enemy_life_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster, u8 weapon_id);
	s32 get_total_ally_life_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster, u8 weapon_id);
	s32 get_total_enemy_shields_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster);
	s32 get_total_enemy_energy_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster);
	s32 get_total_enemy_nuke_value_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster);
}