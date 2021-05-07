#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	u16 statdata_UnitWireframeUpdate_Fragment(CUnit* unit);
	u8 wireframeShields(CUnit* unit);
	//The injector function that should be called in initialize.cpp
	void injectWireframeHook();

} //hooks
