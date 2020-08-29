#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace {
	// For getResponseSFX
	int randFunc();

	// For getRandYesSFX
	int randGetSFXBetween(int a1, int a2);
}

namespace hooks {
	int getResponseSfx(s32 unitId, int a2) { // 0048EA20
		int sound = 0;			//v2
		int whatStart;			//v3
		int whatEnd;			//v4
		int rand;				//v6
		int result;

		/*if (!(units_dat::GroupFlags[unitId].isBuilding)) {		// original check was unitId <= 0x69, now checks for existance of annoyed lines
			scbw::printText("unit has piss lines");
			scbw::printFormattedText("%i", units_dat::SoundAnnoyedFirst[unitId]);
			sound = units_dat::SoundAnnoyedFirst[unitId];
			if ((s16)sound) {
				sound = (u16)sound;
				if (a2 <= (u16)units_dat::SoundAnnoyedLast[unitId] - (u16)sound + 1) {
					if (a2)
						scbw::printText("return sound");
						return sound + a2 - 1;
				}
				else {
					scbw::printText("selectionSoundCounter = 0");
					*selectionSoundCounter = 0;
				}
			}
		}*/
		whatStart = units_dat::SoundWhatFirst[unitId];
		whatEnd = (u16)units_dat::SoundWhatLast[unitId];
		if (whatStart == whatEnd) {
			*lastRandSFX = units_dat::SoundWhatFirst[unitId];
			result = whatStart;
		}
		else {
			rand = whatStart + (randFunc() >> 8) % (whatEnd - whatStart + 1);
			if (rand == *lastRandSFX && ++rand > whatEnd)
				rand = whatStart;
			*lastRandSFX = rand;
			result = rand;
		}
		return result;
	}

	int getRandYesSFX(s32 a1) { // 0048EAB0
		u16 v1; // ax
		int result; // eax

		if (!(units_dat::GroupFlags[a1].isBuilding) && (v1 = units_dat::SoundYesFirst[a1]) != 0)
			result = randGetSFXBetween((int)units_dat::SoundYesLast[a1], (int)v1);
		else
			result = 0;
		return result;
	}
}

namespace {
	// For getResponseSFX
	const u32 Func_randFunc = 0x0040C7BD;
	int randFunc() {
		static int result;
		__asm {
			PUSHAD
			CALL Func_randFunc
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// For getRandYesSFX
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
	}
}