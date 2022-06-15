#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 getSeekRangeHook(Unit* unit, Unit* target);
	u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId, Unit* target);
	void inject_weapon_range_hooks();
}