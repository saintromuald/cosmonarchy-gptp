#include "build_time.h"
#include "SCBW/api.h"
#include "SCBW/UnitFinder.h"

namespace hooks {

	class othstolethMatch : public scbw::UnitFinderCallbackMatchInterface {
	private:
		int _playerId;

	public:

		othstolethMatch(int playerId) {
			_playerId = playerId;
		}

		bool match(CUnit* unit) {
			if (unit->playerId != _playerId && !scbw::isAlliedTo(_playerId, unit->playerId)) return false;
			if (unit->id != UnitId::ZergOthstoleth) return false;
			return true;
		}
	};

	s32 decrementRemainingBuildTime(CUnit* unit) { // 0x00466940
		s16 remainingBuildTime = unit->remainingBuildTime;
		if (!remainingBuildTime) {
			return 1;
		}

		int multiplier = 1;
		if (scbw::isCheatEnabled(CheatFlags::OperationCwal)) {
			multiplier = 16;
		}

		bool othstolethFound = false;
		int radius = 224 / 2;
		if (scbw::UnitFinder(unit->position.x - radius, unit->position.y - radius,
			unit->position.x + radius, unit->position.y + radius).getFirst(othstolethMatch(unit->playerId)) != NULL) {
			othstolethFound = true;
		}

		if (othstolethFound && *elapsedTimeFrames % 2 == 0) {
			unit->remainingBuildTime = remainingBuildTime - 2 * multiplier;
		} else {
			unit->remainingBuildTime = remainingBuildTime - 1 * multiplier;
		}
		if (unit->remainingBuildTime <= 0) {
			unit->remainingBuildTime = 0;
		}

		return 0;
	}

}
