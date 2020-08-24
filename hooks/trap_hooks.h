#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks {
	bool unitIsFloorTrap(int a1); // 0047B070
	s16 disableUnit(CUnit* pUnit); // 0047B4B0
	u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* a1); // 0049FA40

	void injectTrapHooks();
}