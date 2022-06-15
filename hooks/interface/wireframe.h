#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	u16 stat_data_unit_wireframe_update_fragment(Unit* unit);
	u8 wireframe_shields(Unit* unit);
	//The injector function that should be called in initialize.cpp
	void inject_wireframe_hooks();

} //hooks
