#include "SCBW/api.h"
#include "SCBW/scbwdata.h"

namespace hooks
{
	bool unitIsFloorTrap(Unit* unit); // 0047B070
	s16 disableTrap(Unit* pUnit); // 0047B4B0
	u32 updateUnitStrengthAndApplyDefaultOrdersHook(Unit* unit); // 0049FA40
	bool unitIsInstallationAsset(u16 id);
	void injectTrapHooks();
}