#pragma once
#include "hooks/limits/extender.h"

static uint32 NEW_SOUND_TYPE_COUNT = 1144;
//BG
unknown_byte* makeCleanSFXDAT_Unknown(int count);
extern uint8* SFXDAT_Volume;
extern uint8* SFXDAT_Flags;
extern uint16* SFXDAT_Race;
extern uint8* SFXDAT_Type;
extern uint32* SFXDAT_SoundFile;
extern unknown_byte* SFXDAT_Unknown;

namespace hooks {
	void sfxExtenderGetCount();
	void newSfxExtender();
}