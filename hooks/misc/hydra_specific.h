#pragma once
//
#include <hook_tools.h>
#include <SCBW/api.h>
#include <SCBW/enumerations.h>
#include <SCBW/scbwdata.h>

namespace hooks {
	u32 realTopSpeed(CFlingy* flingy, u32 flingyId);
	u16 realAcceleration(CFlingy* flingy, u32 flingyId);
	bool is_high_templar_conduit(u16 unitId);
	u32 weapon_flingy(u32 weaponId, u8 playerId);
	u32 getFlingyHaltDistance(CFlingy* flingy);
}

namespace hooks {
	void injectHydraSpecific();

}