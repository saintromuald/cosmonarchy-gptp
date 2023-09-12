#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{
	s32 unit_movement_dormant(Unit* unit);								// 0046A8D0
	void move_unit(Unit* unit, s16 x, s16 y, bool ignore_collision);	// 004EB9F0
	bool can_collide_with(Unit* unit, Unit* colliding_unit);
	s32 unit_movement_start_path(Unit* unit);							// 0046BD90

	void inject_unit_movement_hooks();
}