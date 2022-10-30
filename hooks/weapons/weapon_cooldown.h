#pragma once
#include "../../SCBW/structures.h"

namespace hooks
{
	u32 get_modified_armor_pen(Unit* unit, u8 weapon_id);

	u32 get_modified_cooldown(Unit* unit, u32 cooldown, bool clamp);
	u32 get_modified_weapon_cooldown(Unit* unit, u8 weapon_id);
	f32 get_weapon_cooldwon_float(Unit* unit, u8 weapon_id);
	f32 get_current_dps(Unit* unit, u8 weapon_id);
	f32 get_current_dps_from_damage(Unit* unit, u8 weapon_id, u32 damage);

	void inject_weapon_cooldown_hooks();
}