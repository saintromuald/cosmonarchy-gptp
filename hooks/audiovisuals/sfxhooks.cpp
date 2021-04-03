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
	int getResponseSfx(int32 unitId, int a2) { // 0048EA20
		int sound = 0;			//v2
		int whatStart;			//v3
		int whatEnd;			//v4
		int rand;				//v6
		int result;

		if (!(units_dat::GroupFlags[unitId].isBuilding)) {		// original check was unitId <= 0x69, now checks for existance of annoyed lines
			sound = scbw::getUnitData(UnitsDatEntries::SndPssFirst,unitId);
			if ((int16)sound) {
				sound = (uint16)sound;
				if (a2 <= (uint16)scbw::getUnitData(UnitsDatEntries::SndPssLast,unitId)-(uint16)sound+1){
					if (a2) {
						return sound + a2 - 1;
					}
				}
				else {
					*selectionSoundCounter = 0;
				}
			}
		}
		whatStart = scbw::getUnitData(UnitsDatEntries::SndWhtFirst, unitId);
		whatEnd = (uint16)scbw::getUnitData(UnitsDatEntries::SndWhtLast, unitId);
		if (whatStart == whatEnd) {
			*lastRandSFX = scbw::getUnitData(UnitsDatEntries::SndWhtFirst, unitId);
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

	int getRandYesSFX(int32 a1) { // 0048EAB0
		uint16 v1; // ax
		int result; // eax

		if (!(units_dat::GroupFlags[a1].isBuilding) && (v1 = scbw::getUnitData(UnitsDatEntries::SndYesFirst,a1)) != 0)
			result = randGetSFXBetween((int)scbw::getUnitData(UnitsDatEntries::SndYesLast,a1), (int)v1);
		else
			result = 0;
		return result;
	}
}

namespace {
	// For getResponseSFX
	const uint32 Func_randFunc = 0x0040C7BD;
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
	const uint32 Func_randGetSFXBetween = 0x0048E940;
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