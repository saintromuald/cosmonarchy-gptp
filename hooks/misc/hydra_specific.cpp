//
#include "hooks/misc/hydra_specific.h"

namespace hooks {
	u32 realTopSpeed(CFlingy* flingy, u32 flingyId) {
		u32 speed = scbw::getFlingyData(FlingyDatEntries::TopSpeed, flingyId);
		if (flingy != NULL && flingy->sprite!=NULL) {
			if (flingy->sprite->spriteId == SpriteId::MadrigalMissile2) {
				if (scbw::getUpgradeLevel(flingy->sprite->playerId, UpgradeId::RequiemAfterburners)) {
					speed += speed / 2;
				}
			}
		}
		return speed;
	}
	bool is_high_templar_conduit(u16 unitId) {
		switch (unitId) {
		case UnitId::ProtossArchon:
		case UnitId::ProtossAugur:
			return true;
		default:
			break;
		}
		return false;
	}
}
