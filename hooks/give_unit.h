#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

void GiveUnit(CUnit* unit, uint32 playerId, uint32 unkScore);	//0049EFA0

void injectGiveUnitHook();

} //hooks