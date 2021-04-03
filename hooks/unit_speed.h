#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

uint32 getModifiedUnitSpeedHook(CUnit* unit, uint32 baseSpeed);
uint32 getModifiedUnitAccelerationHook(CUnit* unit);
uint32 getModifiedUnitTurnSpeedHook(CUnit* unit);

void injectUnitSpeedHooks();

} //hooks
