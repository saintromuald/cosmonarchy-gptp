#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u8 getSeekRangeHook(CUnit* unit);
u32 getMaxWeaponRangeHook(CUnit* unit, u8 weaponId);

void injectWeaponRangeHooks();

} //hooks
