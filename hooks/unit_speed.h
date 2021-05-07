#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

u32 getModifiedUnitSpeedHook(CUnit* unit, u32 baseSpeed);
u32 getModifiedUnitAccelerationHook(CUnit* unit);
u32 getModifiedUnitTurnSpeedHook(CUnit* unit);

void injectUnitSpeedHooks();

} //hooks
