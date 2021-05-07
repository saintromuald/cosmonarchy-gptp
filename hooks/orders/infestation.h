#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	bool isInfestableUnitHook(Unit* unit);
	bool unitCanInfestHook(Unit* unit);

	//infestation order used by infestation victim
	void orderMorphIntoInfested(Unit* unitInfested);

	//infestation order used by the unit that infest
	void orderInfestTarget(Unit* unitInfesting);

	//The injector function that should be called in initialize.cpp
	void injectInfestationHooks();

} //hooks
