#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

u32 getModifiedUnitSpeedHook(Unit* unit, u32 baseSpeed);
u32 getModifiedUnitAccelerationHook(Unit* unit);
u32 getModifiedUnitTurnSpeedHook(Unit* unit);

void injectUnitSpeedHooks();

} //hooks
