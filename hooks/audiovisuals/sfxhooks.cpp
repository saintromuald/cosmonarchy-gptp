#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace {

}

namespace hooks {
	int getResponseSfx(int unitId, int a2) {
		int sound;				//v2
		int whatStart;			//v3
		int whatEnd;			//v4
		int rand;				//v6
		int result;				

		if (units_dat::GroupFlags[unitId].isBuilding) {		// original check was unitId <= 0x69, now checks for building flag
			sound = scbw::SoundAnnoyedFirst[unitId];
			if (sound) {
				if (a2 <= scbw::SoundAnnoyedLast[unitId] - sound + 1) {
					if (a2) {
						return sound + a2 - 1;
					}
					else {
						scbw::selectionSoundCounter = 0;
					}
				}
			}
		}
		whatStart = scbw::SoundWhatFirst[unitId];
		whatEnd = scbw::SoundWhatLast[unitId];
		if (whatStart == whatEnd) {
			scbw::lastRandSFX = scbw::SoundWhatFirst[unitId];
			result = whatStart;
		}
		else {
			rand = scbw::randBetween(whatStart, whatEnd);
			if (rand == scbw::lastRandSFX && ++rand > whatEnd) {
				rand = whatStart;
			}
			scbw::lastRandSFX = rand;
			result = rand;
		}
		return result;
	}
};

namespace {

}