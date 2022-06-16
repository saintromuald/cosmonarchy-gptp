#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	const char* get_weapon_tooltip_string(u8 weaponId, Unit* unit);
	const char* get_armor_tooltip_string(Unit* unit);
	const char* get_shield_tooltip_string(Unit* unit);
	const char* get_special_tooltip_string(u16 iconUnitId, Unit* unit);

	void inject_unit_tooltip_hooks();
}