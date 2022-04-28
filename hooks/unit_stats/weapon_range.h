#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 getSeekRangeHook(Unit* unit);
	u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId);

	s32 get_standard_attack_target(Unit* target, Unit* attacker);		// 00442DA0
	void injectWeaponRangeHooks();
}