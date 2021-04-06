#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

	uint8 getCloakingTech(CUnit* unit);

	bool32 BTNSCOND_IsCloaked(CUnit* unit, uint32 playerId, uint16 reqVar);	//0x00429210
	bool32 BTNSCOND_CanCloak(CUnit* unit, uint32 playerId, uint16 reqVar);	//0x004292C0

	void injectCloakingTechHooks();

} //hooks
