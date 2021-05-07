#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_TerranBuildSelf(CUnit* building);		// 00467760
	void orders_SCVBuild2(CUnit* unit);					// 00467A70
	void orders_SCVBuild(CUnit* unit);					// 00467FD0
	s32 updatePlacementStateForPlacementBox();		// 0048DCE0
	char refreshScreen(s32 ecx0);						// 0048DDC0

	void injectBuildingTerranHook();

} //hooks