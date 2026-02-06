#pragma once
#include "../BW/structures/Unit.h"

namespace hooks {
f64 get_speed_multiplier(Unit* unit);
u32 get_modified_unit_speed(Unit* unit, u32 base_speed);
u32 get_modified_unit_acceleration(Unit* unit);
u32 get_modified_unit_turn_speed(Unit* unit);
Unit* set_unit_movement_speed(Unit* unit, u32 speed);		// 004951C0
void update_iscript_speed(Unit* unit);						// 0042D8C0
s32 get_flingy_halt_distance(Unit* unit);					// 00494F90

void inject_unit_speed_hooks();
} // namespace hooks