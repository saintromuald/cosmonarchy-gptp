#pragma once
#include <BW/structures/Unit.h>

namespace hooks {
	s32 get_bonus_armor(Unit* unit, u8 base_armor);

	const char* get_weapon_tooltip_string(WeaponId::Enum weapon_id, Unit* unit);
	const char* get_armor_tooltip_string(Unit* unit);
	const char* get_shield_tooltip_string(Unit* unit);
	const char* get_special_tooltip_string(u16 icon_unit_id, Unit* unit);	

	void inject_unit_tooltip_hooks();
}