#pragma once
#include "hooks/limits/extender.h"

static u32 NEW_SOUND_TYPE_COUNT = 1144;

namespace sfx_dat {
	extern u8* volume;
	extern u8* sound_flags;
	extern u16* portrait_duration;
	extern u8* type;
	extern char** sound_file;
	extern SoundData* sound_data;
}

namespace hooks
{
	void sfx_extender_get_count();
	void new_sfx_extender();
}