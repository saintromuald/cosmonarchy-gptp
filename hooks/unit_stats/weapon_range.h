#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 getSeekRangeHook(Unit* unit, Unit* target);
	u32 get_max_weapon_range(Unit* unit, u8 weapon_id, Unit* target);
	void inject_weapon_range_hooks();
}