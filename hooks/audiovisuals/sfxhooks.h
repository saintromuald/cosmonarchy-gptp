#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	u32 get_unit_response_id(s32 a1, s32 last_sound_id);					// 0048EA20
	void play_yes_unit_response(Unit* self);								// 0048F4D0
	u32 get_random_yes_unit_response_internal(u32 unit_id, u32 order_id);	// 0048EAB0
	u32 get_random_yes_unit_response(u32 unit_id);							// 0048EAB0
	void play_unit_ready_sound(Unit* unit);									// 0048F440
	void inject_sfx_hooks();
}