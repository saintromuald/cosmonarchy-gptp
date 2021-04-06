#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	uint16 statdata_UnitWireframeUpdate_Fragment(CUnit* unit);
	uint8 wireframeShields(CUnit* unit);
	//The injector function that should be called in initialize.cpp
	void injectWireframeHook();

} //hooks
