#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	u8 getCloakingTech(CUnit* unit);

	b32 BTNSCOND_IsCloaked(CUnit* unit, u32 playerId, u16 reqVar);	//0x00429210
	b32 BTNSCOND_CanCloak(CUnit* unit, u32 playerId, u16 reqVar);	//0x004292C0

	void injectCloakingTechHooks();

} //hooks
