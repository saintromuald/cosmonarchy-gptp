#pragma once
#include <BW/structures/Unit.h>

namespace hooks
{
	u8 get_seek_range(Unit* unit, Unit* target);
	u32 get_max_weapon_range(Unit* unit, u8 weapon_id, Unit* target, bool ignore_cliff_advantage = false);
	void inject_weapon_range_hooks();
}