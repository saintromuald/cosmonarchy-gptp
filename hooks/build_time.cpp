#include "build_time.h"
#include "SCBW/api.h"
#include "SCBW/UnitFinder.h"
#include "globals.h"

namespace hooks
{
	s32 decrementRemainingBuildTime(Unit* unit) // 0x00466940
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		if (!unit->remainingBuildTime)
		{
			return 1;
		}
		u16 multiplier = 1;
		if (scbw::isCheatEnabled(CheatFlags::OperationCwal))
		{
			multiplier = 16;
		}
		if (unitExt->timers[TimerEggBoost] % 2 == 1)
		{
			multiplier *= 2;
		}
		unit->remainingBuildTime -= std::min(multiplier, unit->remainingBuildTime);
		return 0;
	}
}
