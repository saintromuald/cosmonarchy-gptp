#pragma once
#include "../BW/structures/Unit.h"

namespace hooks
{
	u8 get_cloaking_tech(Unit* unit);										// 00491A50
	b32 btnscond_is_cloaked(Unit* unit, u32 player, u16 requirement_var);	// 00429210
	b32 btnscond_can_cloak(Unit* unit, u32 player, u16 requirement_var);	// 004292C0

	void inject_cloaking_tech_hooks();
}