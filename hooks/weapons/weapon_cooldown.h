#pragma once
#include "../../SCBW/structures.h"

namespace hooks {

uint32 getModifiedWeaponCooldownHook(CUnit* unit, uint8 weaponId);

void injectWeaponCooldownHook();

} //hooks
