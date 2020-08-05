#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace {
	int randGetSFXBetween(int a1, int a2);
}

namespace hooks {
	int getResponseSfx(signed int unitId, int a2) { // 0048EA20
		int sound;				//v2
		int whatStart;			//v3
		int whatEnd;			//v4
		int rand;				//v6
		int result;

		if (units_dat::GroupFlags[unitId].isBuilding) {		// original check was unitId <= 0x69, now checks for building flag
			ByteOperations::SetLowWord(sound, units_dat::SoundAnnoyedFirst[unitId]);
			if (sound) {
				if (a2 <= units_dat::SoundAnnoyedLast[unitId] - sound + 1) {
					if (a2) {
						return sound + a2 - 1;
					}
					else {
						*selectionSoundCounter = 0;
					}
				}
			}
		}
		whatStart = units_dat::SoundWhatFirst[unitId];
		whatEnd = units_dat::SoundWhatLast[unitId];
		if (whatStart == whatEnd) {
			*lastRandSFX = units_dat::SoundWhatFirst[unitId];
			result = whatStart;
		}
		else {
			rand = scbw::randBetween(whatStart, whatEnd);
			if (rand == (int)lastRandSFX && ++rand > whatEnd) {
				rand = whatStart;
			}
			*lastRandSFX = rand;
			result = rand;
		}
		return result;
	};

	int getRandYesSFX(signed int a1) { // 0048EAB0
		unsigned __int16 v1; // ax
		int result; // eax

		if (!(units_dat::GroupFlags[a1].isBuilding) && (v1 = units_dat::SoundYesFirst[a1]) != 0)
			result = randGetSFXBetween((unsigned __int16)units_dat::SoundYesFirst[a1], v1);
		else
			result = 0;
		return result;
	};
};

namespace {
	const u32 Func_randGetSFXBetween = 0x0048E940;
	int randGetSFXBetween(int a1, int a2) {
		static int result;

		__asm {
			PUSHAD
			MOV EDI, a1
			MOV ESI, a2
			CALL Func_randGetSFXBetween
			MOV result, EAX
			POPAD
		}
		return result;
	};
}