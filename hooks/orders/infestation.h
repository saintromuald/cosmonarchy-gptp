#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks
{

	bool unit_can_be_infested(Unit* unit);
	bool unit_can_infest(Unit* unit);

	//infestation order used by infestation victim
	void unit_morph_into_infested_order(Unit* unitInfested);

	//infestation order used by the unit that infest
	void unit_infest_order(Unit* unitInfesting);

	//The injector function that should be called in initialize.cpp
	void inject_infestation_hooks();

} //hooks
