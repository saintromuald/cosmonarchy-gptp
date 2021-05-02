#pragma once
#include "hooks/limits/extender.h"

static u32 NEW_SOUND_TYPE_COUNT = 1144;
//BG
unk_t* makeCleanSFXDAT_Unknown(s32 count);
extern u8* SFXDAT_Volume;
extern u8* SFXDAT_Flags;
extern u16* SFXDAT_Race;
extern u8* SFXDAT_Type;
extern u32* SFXDAT_SoundFile;
extern unk_t* SFXDAT_Unknown;

namespace hooks {
	void sfxExtenderGetCount();
	void newSfxExtender();
}