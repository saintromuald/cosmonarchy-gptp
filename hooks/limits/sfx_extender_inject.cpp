#include "hooks/limits/sfx_extender.h"
#include "globals.h"

unknown_byte* makeCleanSFXDAT_Unknown(int count) {
	unknown_byte* _Unknown = new unknown_byte[count];
	for (int x = 0; x < count; ++x)
		_Unknown[x] = NULL;
	return _Unknown;
}

uint8* SFXDAT_Volume;
uint8* SFXDAT_Flags;
uint16* SFXDAT_Race;
uint8* SFXDAT_Type;
uint32* SFXDAT_SoundFile;
unknown_byte* SFXDAT_Unknown;

namespace hooks {
	void sfxExtenderGetCount() {
		Globals.extended_sfx = 1;
		//        auto sfx_data = scbw::readSamaseFile("arr\\sfxdata.dat");
	}
	void newSfxExtender() {
		NEW_SOUND_TYPE_COUNT = 20000;   //Define your length here (change later to custom func reading sfxdata)
		SFXDAT_Volume = new uint8[NEW_SOUND_TYPE_COUNT];
		SFXDAT_Flags = new uint8[NEW_SOUND_TYPE_COUNT];
		SFXDAT_Race = new uint16[NEW_SOUND_TYPE_COUNT];
		SFXDAT_Type = new uint8[NEW_SOUND_TYPE_COUNT];
		SFXDAT_SoundFile = new uint32[NEW_SOUND_TYPE_COUNT];
		SFXDAT_Unknown = makeCleanSFXDAT_Unknown(NEW_SOUND_TYPE_COUNT * 16);

		memoryPatch(0x0045F042, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x004BBC2D, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x004BBCA5, NEW_SOUND_TYPE_COUNT - 1);
		memoryPatch(0x004BBEB7, NEW_SOUND_TYPE_COUNT - 1);
		memoryPatch(0x004BCA98, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x004BCD2A, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x004DB0FA, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x005154A0, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x005154AC, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x005154B8, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x005154C4, NEW_SOUND_TYPE_COUNT);
		memoryPatch(0x005154D0, NEW_SOUND_TYPE_COUNT);

		memoryPatch(0x00455BDC, SFXDAT_Volume);
		memoryPatch(0x0048D0F0, ((uint32)SFXDAT_Volume) + 0x17);
		memoryPatch(0x0048D16C, ((uint32)SFXDAT_Volume) + 2);
		memoryPatch(0x0048D409, ((uint32)SFXDAT_Volume) + 0x17);
		memoryPatch(0x0048ECED, SFXDAT_Volume);
		memoryPatch(0x0048EDD7, SFXDAT_Volume);
		memoryPatch(0x0048EE5C, SFXDAT_Volume);
		memoryPatch(0x0048EFA5, SFXDAT_Volume);
		memoryPatch(0x0048F028, SFXDAT_Volume);
		memoryPatch(0x0048F0CA, SFXDAT_Volume);
		memoryPatch(0x0048F1A7, SFXDAT_Volume);
		memoryPatch(0x004E3EB5, SFXDAT_Volume);
		memoryPatch(0x004E3F55, SFXDAT_Volume);
		memoryPatch(0x005154C8, SFXDAT_Volume);
		memoryPatch(0x004BB972, SFXDAT_Flags);
		memoryPatch(0x004BBC54, SFXDAT_Flags);
		memoryPatch(0x004BBC67, SFXDAT_Flags);
		memoryPatch(0x004BBF17, SFXDAT_Flags);
		memoryPatch(0x004BBF20, SFXDAT_Flags);
		memoryPatch(0x004BBFF3, SFXDAT_Flags);
		memoryPatch(0x004BC06C, SFXDAT_Flags);
		memoryPatch(0x004BC0C0, SFXDAT_Flags);
		memoryPatch(0x004BC7AE, SFXDAT_Flags);
		memoryPatch(0x004BC80A, SFXDAT_Flags);
		memoryPatch(0x004BC846, SFXDAT_Flags);
		memoryPatch(0x004BCAD6, SFXDAT_Flags);
		memoryPatch(0x004BCAF3, SFXDAT_Flags);
		memoryPatch(0x004BCB0B, SFXDAT_Flags);
		memoryPatch(0x004BCD44, SFXDAT_Flags);
		memoryPatch(0x005154B0, SFXDAT_Flags);
		memoryPatch(0x0048F127, SFXDAT_Race);
		memoryPatch(0x0048F204, SFXDAT_Race);
		memoryPatch(0x0048F2EB, SFXDAT_Race);
		memoryPatch(0x0048F376, ((uint32)SFXDAT_Race) + 0x10E);
		memoryPatch(0x0048F3D2, SFXDAT_Race);
		memoryPatch(0x0048F41B, ((uint32)SFXDAT_Race) + 0x10E);
		memoryPatch(0x0048F4A0, SFXDAT_Race);
		memoryPatch(0x0048F572, SFXDAT_Race);
		memoryPatch(0x0048F607, SFXDAT_Race);
		memoryPatch(0x0048F697, SFXDAT_Race);
		memoryPatch(0x0048F732, SFXDAT_Race);
		memoryPatch(0x0048F822, SFXDAT_Race);
		memoryPatch(0x0048F8E3, ((uint32)SFXDAT_Race) + 0x104);
		memoryPatch(0x0048FAEB, SFXDAT_Race);
		memoryPatch(0x005154BC, SFXDAT_Race);
		memoryPatch(0x004BBEC3, SFXDAT_Type);
		memoryPatch(0x004BC0D3, SFXDAT_Type);
		memoryPatch(0x004BC813, SFXDAT_Type);
		memoryPatch(0x005154A4, SFXDAT_Type);
		memoryPatch(0x0046102D, ((uint32)SFXDAT_SoundFile) + 0x58);
		memoryPatch(0x0046116D, ((uint32)SFXDAT_SoundFile) + 0x54);
		memoryPatch(0x004BC690, SFXDAT_SoundFile);
		memoryPatch(0x004BCA3C, SFXDAT_SoundFile);
		memoryPatch(0x004BCB2A, ((uint32)SFXDAT_SoundFile) + 0x3C);
		memoryPatch(0x004DB0E6, SFXDAT_SoundFile);
		memoryPatch(0x00515498, SFXDAT_SoundFile);
		memoryPatch(0x004BB8E8, SFXDAT_Unknown);
		memoryPatch(0x004BBC47, SFXDAT_Unknown);
		memoryPatch(0x004BC077, SFXDAT_Unknown);
		memoryPatch(0x004BC674, SFXDAT_Unknown);
		memoryPatch(0x004BCAA6, SFXDAT_Unknown);
		memoryPatch(0x0045EF66, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F11D, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F1FA, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F2DE, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F3CA, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F498, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F56A, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F5FF, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F68F, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F72A, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F81A, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048FAE3, ((uint32)SFXDAT_Unknown) + 8);
		memoryPatch(0x0048F8DA, ((uint32)SFXDAT_Unknown) + 0x828);
		memoryPatch(0x0048F36F, ((uint32)SFXDAT_Unknown) + 0x878);
		memoryPatch(0x0048F412, ((uint32)SFXDAT_Unknown) + 0x878);
	}
}