#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	u8 get_armor_bonus(Unit* unit); // 00453FC0
	s32 get_armor_internal(Unit* unit);

	void inject_armor_bonus_hooks();
}