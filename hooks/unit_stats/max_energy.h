#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u16 getUnitMaxEnergyHook(Unit* const unit);

void inject_unit_max_energy_hooks();

} //hooks
