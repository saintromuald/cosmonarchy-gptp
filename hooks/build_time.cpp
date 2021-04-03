#include "build_time.h"
#include "SCBW/api.h"
#include "SCBW/UnitFinder.h"
#include "globals.h"

namespace hooks {

	int32 decrementRemainingBuildTime(CUnit* unit) { // 0x00466940
		if (!unit->remainingBuildTime) {
			return 1;
		}
		uint16 multiplier = 1;
		if (scbw::isCheatEnabled(CheatFlags::OperationCwal)) {
			multiplier = 16;
		}
		if (scbw::get_generic_timer(unit, ValueId::EggBoostTimer) % 2 == 1) {
			multiplier *= 2;
		}
		unit->remainingBuildTime -= std::min(multiplier,unit->remainingBuildTime);
		return 0;
	}

}
