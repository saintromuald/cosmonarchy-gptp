#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	u8 getCloakingTech(Unit* unit);

	b32 BTNSCOND_IsCloaked(Unit* unit, u32 player, u16 requirement_var);	//0x00429210
	b32 BTNSCOND_CanCloak(Unit* unit, u32 player, u16 requirement_var);	//0x004292C0

	void inject_cloaking_tech_hooks();

} //hooks
