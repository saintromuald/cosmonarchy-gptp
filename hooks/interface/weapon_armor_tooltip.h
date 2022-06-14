#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	const char* getWeaponTooltipString(u8 weaponId, Unit* unit);
	const char* getArmorTooltipString(Unit* unit);
	const char* getShieldTooltipString(Unit* unit);
	const char* getSpecialTooltipString(u16 iconUnitId, Unit* unit);

	void inject_unit_tooltip_hooks();
}