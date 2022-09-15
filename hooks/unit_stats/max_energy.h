#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {

u16 get_unit_max_energy(Unit* const unit);

void inject_unit_max_energy_hooks();

} //hooks
