#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u16 getUnitMaxEnergyHook(CUnit* const unit);

void injectUnitMaxEnergyHook();

} //hooks
