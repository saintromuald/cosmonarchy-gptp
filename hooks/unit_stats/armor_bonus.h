#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 getArmorBonusHook(Unit* unit);
	void inject_armor_bonus_hooks();
}
