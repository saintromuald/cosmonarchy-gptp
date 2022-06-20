#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_building_land(Unit* unit);			// 00464360
	void orders_building_liftoff(Unit* unit);		// 

	void inject_lift_land_hooks();
}