#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void unit_selection_click(Unit* clicked_unit);
	u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds);

	void inject_selection_hooks();

}
