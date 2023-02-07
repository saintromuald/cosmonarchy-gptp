#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting);

	void inject_sight_range_hooks();
}