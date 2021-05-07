#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	u16 statdata_UnitWireframeUpdate_Fragment(Unit* unit);
	u8 wireframeShields(Unit* unit);
	//The injector function that should be called in initialize.cpp
	void injectWireframeHook();

} //hooks
