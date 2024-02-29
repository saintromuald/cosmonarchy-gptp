#pragma once
#include <BW/structures/Unit.h>

const s32 DEFAULT_SIGHT_RANGE = 4;
const s32 BLIND_SIGHT_RANGE = 2;

namespace hooks {
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting);											// 004E5B40

	void inject_sight_range_hooks();
}