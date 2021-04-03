#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks {
	bool unitIsFloorTrap(CUnit* unit); // 0047B070
	int16 disableUnit(CUnit* pUnit); // 0047B4B0
	uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit); // 0049FA40
	bool unitIsInstallationAsset(uint16 id);
	void injectTrapHooks();
}