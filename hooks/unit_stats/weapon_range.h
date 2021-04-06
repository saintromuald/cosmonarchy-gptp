#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

uint8 getSeekRangeHook(CUnit* unit);
uint32 getMaxWeaponRangeHook(CUnit* unit, uint8 weaponId);

void injectWeaponRangeHooks();

} //hooks
