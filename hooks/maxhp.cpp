#include "maxhp.h"
#include "SCBW\api.h"
#include "SCBW\scbwdata.h"
#include "hook_tools.h"

namespace hooks {
	int maxhp(int id, int playerId) {
		int result = units_dat::MaxHitPoints[id];
		/*
		if(scbw::getUpgradeLevel(playerId,UpgradeId::Heatshield)){
			result+=15*256;
		}
		*/
		return result;
	}
}