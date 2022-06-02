#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

f64 get_speed_multiplier(Unit* unit);
u32 get_modified_unit_speed(Unit* unit, u32 baseSpeed);
u32 get_modified_unit_acceleration(Unit* unit);
u32 get_modified_unit_turn_speed(Unit* unit);

void injectUnitSpeedHooks();

} //hooks
