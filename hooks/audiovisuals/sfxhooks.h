#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	u32 get_unit_response_id(s32 a1, s32 last_sound_id);		// 0048EA20
	s32 get_random_yes_unit_response(s32 unit_id);				// 0048EAB0
	void inject_sfx_hooks();
}