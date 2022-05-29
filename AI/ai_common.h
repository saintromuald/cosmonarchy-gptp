//Common functions used by the AI.
#pragma once
#include "../SCBW/UnitFinder.h"
#pragma once
#include "../SCBW/structures/Unit.h"
#include "../SCBW/api.h"

namespace AI
{

	bool isTargetWorthHitting(Unit* target, Unit* attacker);

	/// Returns true if @p target is attacking a player allied to @p unit.
	bool isTargetAttackingAlly(Unit* target, Unit* unit);

	/// Determines whether the given @p playerId is in a UMS map (i.e. campaign mode).
	bool isUmsMode(u8 playerId);

	/// Determines if the @p unit is within a region that is not commanded by its
	/// controlling AI. Details are not really understood.
	bool unit_is_in_unsafe_region(Unit* unit);

	s32 getTotalEnemyLifeInArea(s32 x, s32 y, s32 searchBounds, Unit* caster, u8 weaponId);
	s32 getTotalAllyLifeInArea(s32 x, s32 y, s32 searchBounds, Unit* caster, u8 weaponId);
	s32 getTotalEnemyShieldsInArea(s32 x, s32 y, s32 searchBounds, Unit* caster);
	s32 getTotalEnemyEnergyInArea(s32 x, s32 y, s32 searchBounds, Unit* caster);
	s32 getTotalEnemyNukeValueInArea(s32 x, s32 y, s32 searchBounds, Unit* caster);

	//-------- Template function definition --------//

} //AI