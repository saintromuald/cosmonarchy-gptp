#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_BuildingLand(Unit* unit);
	void orders_BuildingLiftoff(Unit* unit);

	//The injector function that should be called in initialize.cpp
	void injectLiftLandHooks();

} //hooks
