#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_BuildingLand(Unit* unit);			// 00464360
	void orders_BuildingLiftoff(Unit* unit);		// 

	void inject_lift_land_hooks();
}