#pragma once
#include "../../SCBW/structures.h"

namespace hooks
{
	u32 get_modified_weapon_cooldown(Unit* unit, u8 weaponId);
	f32 getWeaponCooldownFloat(Unit* unit, u8 weaponId);
	f32 getCurrentDPS(Unit* unit, u8 weaponId);

	void injectWeaponCooldownHook();
}