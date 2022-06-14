#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks
{

	s32 unitMovementDormant(Unit* unit);							// 0046A8D0
	void move_unit(Unit* unit, s16 x, s16 y, bool ignoreCollision);	// 004EB9F0

	void inject_unit_movement_hooks();
}