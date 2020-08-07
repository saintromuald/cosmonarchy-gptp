#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"
#include "../../SCBW/api.h"
#include "../../MathUtils.h"


namespace hooks {

/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
///
/// @return		The modified cooldown value.
u32 getModifiedWeaponCooldownHook(CUnit* unit, u8 weaponId) {


	u32 cooldown = weapons_dat::Cooldown[weaponId];
	u32 adrenalStacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::AdrenalStacks, 0);

	if (unit->acidSporeCount != 0) {

		u32 increaseAmt = cooldown / 8;

		if (increaseAmt < 3) 
			increaseAmt = 3;

		cooldown += increaseAmt * unit->acidSporeCount;

	}
	
	int cooldownModifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
							+ (unit->status & UnitStatus::CooldownUpgrade ? 1 : 0);
	// Capacitors upgrade: gains increased attack speed while cloaked
	if(scbw::getUpgradeLevel(unit->playerId, UpgradeId::Capacitors) && unit->id==UnitId::wraith &&
		weaponId==WeaponId::BurstLasers && (unit->status & UnitStatus::Cloaked)){
		cooldown -= cooldown / 5;
	}
	// Ravening Mandibule upgrade: gains increased attack speed
	if (unit->id==UnitId::ZergGorgoleth && scbw::getUpgradeLevel(unit->playerId, UpgradeId::RaveningMandible)) {
		cooldown *= 0.75;
		cooldown *= MathUtils::map(unit->energy, (u16)100, (u16)250, 1.0, 0.5, true);
	}
	// Kaiser Rampage upgrade: gains double attack speed for 1 second
	if (unit->id == UnitId::ZergUltralisk && scbw::getUpgradeLevel(unit->playerId, UpgradeId::KaiserRampage)) {
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KaiserRampage, 0) > 0) {
			cooldown /= 2;
		}
	}
	// Terminus passive: gains attack speed with each upgrade
	if (unit->id == UnitId::ProtossTerminus && scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossGroundWeapons)) {
		int terminusUpg = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossGroundWeapons);
		cooldown -= cooldown * (0.1 * terminusUpg);
	}

	if (cooldownModifier > 0)
		cooldown /= 2;
	else 
	if (cooldownModifier < 0 ){
		cooldown += cooldown / 4;
		
	}
	if (unit->stimTimer) {
		cooldown += 2;		// Fixes AICE Stimpack issue
	}
	if (adrenalStacks) {
		cooldown -= adrenalStacks;
	}
	if (cooldown > 250)
		cooldown = 250;
	else 
	if (cooldown < 3) 
		cooldown = 3;
	return cooldown;

}

} //hooks
