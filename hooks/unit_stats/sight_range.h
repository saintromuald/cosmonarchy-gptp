#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u32 getSightRangeHook(Unit* unit, bool isForSpellCasting);

void injectSightRangeHook();

} //hooks
