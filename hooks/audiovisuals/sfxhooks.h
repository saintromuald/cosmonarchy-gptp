#pragma once
#include "BW/structures.h"

namespace hooks
{
	u32 get_unit_response_id(s32 a1, s32 last_sound_id);					// 0048EA20
	void play_yes_unit_response(Unit* self);								// 0048F4D0
	u32 get_random_yes_unit_response_local(u32 unit_id);							// 0048EAB0
	void play_unit_ready_sound(Unit* unit);									// 0048F440

	// Internal funcs:
	u32 get_random_yes_unit_response_internal(Unit* unit, UnitResponseContext context);
	void play_yes_unit_response_internal(Unit* self, UnitResponseContext context);

	void inject_sfx_hooks();
}