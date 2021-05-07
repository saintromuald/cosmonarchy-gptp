#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	bool isInfestableUnitHook(CUnit* unit);
	bool unitCanInfestHook(CUnit* unit);

	//infestation order used by infestation victim
	void orderMorphIntoInfested(CUnit* unitInfested);

	//infestation order used by the unit that infest
	void orderInfestTarget(CUnit* unitInfesting);

	//The injector function that should be called in initialize.cpp
	void injectInfestationHooks();

} //hooks
