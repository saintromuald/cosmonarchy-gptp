#pragma once
#include <SCBW/structures/Unit.h>

const s32 DEFAULT_SIGHT_RANGE = 4;
const s32 BLIND_SIGHT_RANGE = 2;

namespace hooks {
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting);											// 004E5B40
	void reveal_sight_at_location(s32 sight_range, u32 flags, s32 x_pos, s32 y_pos, bool is_air_unit);	// 004806F0

	void inject_sight_range_hooks();
}