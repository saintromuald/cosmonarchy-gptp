#include "build_time.h"
#include "SCBW/api.h"
#include "SCBW/UnitFinder.h"

namespace hooks {

	s32 decrementRemainingBuildTime(CUnit* unit) { // 0x00466940
		int remainingBuildTime = unit->remainingBuildTime;
		if (!remainingBuildTime) {
			return 1;
		}
		u16 multiplier = 1;
		if (scbw::isCheatEnabled(CheatFlags::OperationCwal)) {
			multiplier = 16;
		}
		if (scbw::get_aise_value(unit, 0, AiseId::GenericTimer, ValueId::EggBoostTimer, 0) % 2 == 1) {
			multiplier *= 2;
		}
		unit->remainingBuildTime -= std::min(multiplier,unit->remainingBuildTime);
		return 0;
	}

}
