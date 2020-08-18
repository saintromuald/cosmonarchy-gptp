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
	u16 realAcceleration(CFlingy* flingy, u32 flingyId) {
		u16 acceleration = scbw::getFlingyData(FlingyDatEntries::Acceleration, flingyId);
		if (Globals.flingy_init_bullet_id == 22) {
			if (scbw::getUpgradeLevel(Globals.flingy_init_player_id, UpgradeId::RequiemAfterburners)) {
				acceleration += acceleration / 2;
			}
		}
		return acceleration;
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
	u32 getFlingyHaltDistance(CFlingy* flingy) {
		auto speed = flingy->current_speed2;
		if (speed == 0 || flingy->flingyMovementType != 0) {
			return 0;
		}
		auto flingy_id = flingy->flingyId;
		if (speed != realTopSpeed(flingy, flingy_id) || flingy->flingyAcceleration != realAcceleration(flingy,flingy_id)) {
			return speed * speed / (2 * (u16)flingy->flingyAcceleration);
		}
		else {
			return scbw::getFlingyData(FlingyDatEntries::HaltDistance, flingy_id);
		}
	}
	bool is_unused_order(u8 orderId) {
		switch (orderId) {
		case OrderId::GuardianAspect:
		case OrderId::Skyfall://OrderId::AttackTile
		case OrderId::Hover:
		case OrderId::Warpin:
		case OrderId::CTFCOP2:
			return true;
		default:
			break;
		}
		return false;
	}
}
