#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	u32 get_larvae_units_from_list(Unit** unit_list, Unit** output_list);		// 00423190
	void BTSNACT_select_larva();												// 00423930

	void injectSelectLarvaHooks();
}