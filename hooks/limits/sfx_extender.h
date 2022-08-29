#pragma once
#include "hooks/limits/extender.h"

/*
static u32 NEW_SOUND_TYPE_COUNT = 1144;

extern u8* SFXDAT_Volume;
extern u8* SFXDAT_Flags;
extern u16* SFXDAT_Race;
extern u8* SFXDAT_Type;
extern char** SFXDAT_SoundFile;
extern SoundData* SFXDAT_SoundData;
*/

namespace hooks
{
	void sfxExtenderGetCount();
	void newSfxExtender();
}