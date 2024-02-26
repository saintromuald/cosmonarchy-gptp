#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_building_land(Unit* unit);			// 00464360
	void orders_building_liftoff(Unit* unit);		// 004649B0
	void play_landing_interrupt_sound(Unit* unit);		// 0048F5A0

	void inject_lift_land_hooks();
}