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

	// Devourer - Corrosive Spores: lower attack speed of victim
	if (unit->acidSporeCount != 0) {

		u32 increaseAmt = cooldown / 8;

		if (increaseAmt < 3) 
			increaseAmt = 3;

		cooldown += increaseAmt * unit->acidSporeCount;
	}
	
	// Set modifier based on Stim (increase), Ensnare (decrease), and attackspeed upgrades (increase)
	int cooldownModifier = (unit->stimTimer ? 1 : 0) - (unit->ensnareTimer ? 1 : 0)
							+ (unit->status & UnitStatus::CooldownUpgrade ? 1 : 0);

	// Capacitors upgrade: gains increased attack speed while cloaked
	if(unit->id==UnitId::wraith && weaponId==WeaponId::BurstLasers && (unit->status & UnitStatus::Cloaked)){
		cooldown -= cooldown / 5;
	}

	// Ravening Mandible upgrade: gains increased attack speed based on current energy
	if (unit->id==UnitId::ZergGorgoleth) {
		cooldown *= 0.75;
		double value = MathUtils::map(unit->energy / 256, 100, 250, 1.0, 0.5, true);
		cooldown *= value;
	}

	// Kaiser Rampage upgrade: gains double attack speed for 1 second
	if (unit->id == UnitId::ZergUltralisk) {
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KaiserRampage, 0) > 0) {
			cooldown /= 2;
		}
	}

#ifdef DELIVERANCE
	// Terminus passive: gains attack speed with each upgrade
	if (unit->id == UnitId::ProtossTerminus && scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossGroundWeapons)) {
		int terminusUpg = scbw::getUpgradeLevel(unit->playerId, UpgradeId::ProtossGroundWeapons);
		cooldown -= cooldown * (0.1 * terminusUpg);
	}

	u32 astral_tempest_stacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::AstralTempestStacks, 0);
	if (astral_tempest_stacks) {
		cooldown *= 1 - astral_tempest_stacks * 0.05;
	}
#endif

	if (cooldownModifier > 0)	// Lame hardcode for attackspeed upgrades
		cooldown /= 2;
	else 
	if (cooldownModifier < 0 ){	// Presumably, lame hardcode for some slow
		cooldown += cooldown / 4;
		
	}
	if (unit->stimTimer) {		// Fixes aice-related Stimpack issue
		cooldown += 2;
	}
	u32 adrenalStacks = scbw::get_aise_value(unit, NULL, AiseId::GenericValue, ValueId::AdrenalStacks, 0);
	if (adrenalStacks) {
		cooldown -= adrenalStacks / 2;
	}
	if (cooldown > 250)			// Max cutoff
		cooldown = 250;
	else 
	if (cooldown < 3)			// Min cutoff (previously 5)
		cooldown = 3;
	return cooldown;

}

} //hooks
