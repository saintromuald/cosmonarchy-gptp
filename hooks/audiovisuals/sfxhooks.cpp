#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {
	
}

namespace hooks {
	int getResponseSfx(int unitId, int a2) { // 0048EA20
		int sound;				//v2
		int whatStart;			//v3
		int whatEnd;			//v4
		int rand;				//v6
		int result;				

		if (units_dat::GroupFlags[unitId].isBuilding) {		// original check was unitId <= 0x69, now checks for building flag
			sound = units_dat::SoundAnnoyedFirst[unitId];
			if (sound) {
				if (a2 <= units_dat::SoundAnnoyedLast[unitId] - sound + 1) {
					if (a2) {
						return sound + a2 - 1;
					}
					else {
						selectionSoundCounter = 0;
					}
				}
			}
		}
		whatStart = units_dat::SoundWhatFirst[unitId];
		whatEnd = units_dat::SoundWhatLast[unitId];
		if (whatStart == whatEnd) {
			lastRandSFX = units_dat::SoundWhatFirst[unitId];
			result = whatStart;
		}
		else {
			rand = scbw::randBetween(whatStart, whatEnd);
			if (rand == (int)lastRandSFX && ++rand > whatEnd) {
				rand = whatStart;
			}
			lastRandSFX = rand;
			result = rand;
		}
		return result;
	}
};

namespace {

}