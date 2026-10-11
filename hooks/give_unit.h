#pragma once
#include "../BW/structures/Unit.h"

namespace hooks
{
	void give_unit(Unit* unit, u32 player, u32 count_as_completed);		// 0049EFA0
	void give_ai(Unit* unit);											// 00463040

	void inject_give_unit_hooks();
}