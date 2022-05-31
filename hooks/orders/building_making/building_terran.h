#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_terran_build_self(Unit* building);		// 00467760
	void orders_scv_build_2(Unit* unit);				// 00467A70
	void orders_scv_build(Unit* unit);					// 00467FD0
	char refresh_screen(s32 ecx0);						// 0048DDC0

	void injectBuildingTerranHook();
}