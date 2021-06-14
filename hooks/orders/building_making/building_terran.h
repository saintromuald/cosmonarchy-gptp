#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_TerranBuildSelf(Unit* building);		// 00467760
	void orders_SCVBuild2(Unit* unit);					// 00467A70
	void orders_SCVBuild(Unit* unit);					// 00467FD0
	char refreshScreen(s32 ecx0);						// 0048DDC0

	void injectBuildingTerranHook();
}