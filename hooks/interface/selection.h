#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void function_0046FB40(Unit* clicked_unit);
	u32 SortAllUnits(Unit* unit, Unit** unit_list, Unit** units_in_bounds);

	void injectSelectMod();

}
