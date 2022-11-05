#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	//go from Siege Mode to Tank Mode
	void orders_tank_mode(Unit* unit);

	//go from Tank Mode to Siege Mode
	void orders_siege_mode(Unit* unit);

	//The injector function that should be called in initialize.cpp
	void inject_siege_transform_hooks();

} //hooks
