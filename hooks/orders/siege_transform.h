#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	//go from Siege Mode to Tank Mode
	void ordersTankMode(CUnit* unit);

	//go from Tank Mode to Siege Mode
	void ordersSiegeMode(CUnit* unit);

	//The injector function that should be called in initialize.cpp
	void injectSiegeTransformHooks();

} //hooks
