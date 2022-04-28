#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 getSeekRangeHook(Unit* unit, Unit* target);
	u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId, Unit* target);

	s32 get_standard_attack_target(Unit* target, Unit* attacker);		// 00442DA0
	void injectWeaponRangeHooks();
}