#include "maxhp.h"
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "hook_tools.h"

namespace hooks {
	u32 maxhp(int id, int playerId) {
		u32 result = units_dat::MaxHitPoints[id];
		if (scbw::getUpgradeLevel(playerId, UpgradeId::Heatshield)) {
			result += 15 * 256;
		}
		return result;
	}

	u32 maxshields(int id, int playerId) {
		u32 result = units_dat::MaxShieldPoints[id];
		if (scbw::getUpgradeLevel(playerId, UpgradeId::Autovitality)) {
			result += result / 2;
		}
		return result;
	}
}