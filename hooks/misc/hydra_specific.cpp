//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
namespace hooks {
	u32 realTopSpeed(CFlingy* flingy, u32 flingyId) {
		u32 speed = scbw::getFlingyData(FlingyDatEntries::TopSpeed, flingyId);
		if (Globals.flingy_init_bullet_id == 22) {
			if (scbw::getUpgradeLevel(Globals.flingy_init_player_id, UpgradeId::RequiemAfterburners)) {
				speed += speed / 2;
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
	u32 weapon_flingy(u32 weaponId, u8 player_id) {
		Globals.flingy_init_bullet_id = weaponId;
		Globals.flingy_init_player_id = player_id;
		return weapons_dat::FlingyId[weaponId];
	}
}
