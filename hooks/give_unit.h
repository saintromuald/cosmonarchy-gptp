#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void giveUnit(Unit* unit, u32 playerId, u32 bCountAsCompleted);		// 0049EFA0
	void giveAI(Unit* unit);											// 00463040

	void injectGiveUnitHook();
}