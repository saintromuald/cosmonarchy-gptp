#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

uint32 getSightRangeHook(CUnit* unit, bool isForSpellCasting);

void injectSightRangeHook();

} //hooks
