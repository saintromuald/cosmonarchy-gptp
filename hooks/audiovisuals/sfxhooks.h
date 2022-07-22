#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	u32 get_unit_response_id(s32 a1, s32 last_sound_id);		// 0048EA20
	void play_yes_unit_response(Unit* self);					// 0048F4D0
	s32 get_random_yes_unit_response(s32 unit_id, Unit* unit);	// 0048EAB0
	void inject_sfx_hooks();
}