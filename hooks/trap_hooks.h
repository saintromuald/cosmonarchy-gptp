#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks {
	bool unitIsFloorTrap(CUnit* unit); // 0047B070
	s16 disableUnit(CUnit* pUnit); // 0047B4B0
	u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit); // 0049FA40
	bool unitIsInstallationAsset(u16 id);
	void injectTrapHooks();
}