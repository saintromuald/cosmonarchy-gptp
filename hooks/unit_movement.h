#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{

	s32 unitMovementDormant(Unit* unit);		// 0046A8D0
	void injectUnitMovementHooks();
}