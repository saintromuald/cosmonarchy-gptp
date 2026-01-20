#pragma once
#include <BW/structures/Unit.h>

namespace hooks {
	b32 unit_movement_dormant(Unit* unit);								// 0046A8D0
	void move_unit(Unit* unit, s16 x, s16 y, bool ignore_collision);	// 004EB9F0
	bool can_collide_with(Unit* unit, Unit* colliding_unit);			// 0042DEE0
	s32 unit_movement_start_path(Unit* unit);							// 0046BD90
	b32 move_towards(Unit* target, Unit* unit);							// 004EB900
	s32 unit_movement_initialize(Unit* unit);							// 0046C320
	void refresh_unit_after_move(Unit* unit);							// 00494160
	b32 decollide_unit(Unit* unit);										// 004F2DE0

	void inject_unit_movement_hooks();
}