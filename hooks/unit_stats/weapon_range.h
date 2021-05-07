#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u8 getSeekRangeHook(Unit* unit);
u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId);

void injectWeaponRangeHooks();

} //hooks
