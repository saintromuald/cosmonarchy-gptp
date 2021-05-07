#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u16 getUnitMaxEnergyHook(Unit* const unit);

void injectUnitMaxEnergyHook();

} //hooks
