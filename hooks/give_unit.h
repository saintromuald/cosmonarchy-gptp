#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	void GiveUnit(CUnit* unit, u32 playerId, u32 unkScore);	//0049EFA0

	void injectGiveUnitHook();

} //hooks