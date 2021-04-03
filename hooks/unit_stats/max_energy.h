#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

uint16 getUnitMaxEnergyHook(CUnit* const unit);

void injectUnitMaxEnergyHook();

} //hooks
