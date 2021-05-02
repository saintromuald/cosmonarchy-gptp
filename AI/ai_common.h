//Common functions used by the AI.
#pragma once
#include "../SCBW/UnitFinder.h"
#pragma once
#include "../SCBW/structures/CUnit.h"
#include "../SCBW/api.h"

namespace AI
{

    bool isTargetWorthHitting(CUnit* target, CUnit* attacker);

    /// Returns true if @p target is attacking a player allied to @p unit.
    bool isTargetAttackingAlly(CUnit* target, CUnit* unit);

    /// Determines whether the given @p playerId is in a UMS map (i.e. campaign mode).
    bool isUmsMode(u8 playerId);

    /// Determines if the @p unit is within a region that is not commanded by its
    /// controlling AI. Details are not really understood.
    bool isUnitInUnsafeRegion(CUnit* unit);

    s32 getTotalEnemyLifeInArea(s32 x, s32 y, s32 searchBounds, CUnit* caster, u8 weaponId);
    s32 getTotalAllyLifeInArea(s32 x, s32 y, s32 searchBounds, CUnit* caster, u8 weaponId);
    s32 getTotalEnemyShieldsInArea(s32 x, s32 y, s32 searchBounds, CUnit* caster);
    s32 getTotalEnemyEnergyInArea(s32 x, s32 y, s32 searchBounds, CUnit* caster);
    s32 getTotalEnemyNukeValueInArea(s32 x, s32 y, s32 searchBounds, CUnit* caster);

    //-------- Template function definition --------//

} //AI