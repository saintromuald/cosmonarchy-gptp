#pragma once
#include <BW/structures/Unit.h>

namespace hooks {
u8 get_seek_range(Unit* unit, Unit* target);															// 00476000
u32 get_base_weapon_range(WeaponId::Enum weapon_id, u16 unit_id);
u32 get_max_weapon_range(Unit* unit, WeaponId::Enum weapon_id, Unit* target, bool ignore_cliff_advantage = false);	// 00475870
void reassign_target_seek_range();																		// 0049E010
void get_standard_attack_target(Unit* target, Unit* attacker);											// 00442DA0

void inject_weapon_range_hooks();
}