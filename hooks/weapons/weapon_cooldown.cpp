#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"
#include "../../SCBW/api.h"
#include "../../MathUtils.h"
#include "globals.h"

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

	// Vassal Terminal Surge behavior
	if (units_dat::BaseProperty[unit->id] & UnitProperty::RoboticUnit) {
		double stacks = MathUtils::clamp<double>(scbw::get_generic_value(unit, ValueId::TerminalSurge), 0, 10);
		if (stacks > 0) {
			double newCooldown = (double)cooldown * (1.0 - stacks / 13.333);
			cooldown = newCooldown;
		}
	}

	// Madcap Maverick Feeders behavior: increase weapon speed while stationary
	if (unit->id == UnitId::TerranMadcap) {
		u32 stacks = ceil((double)scbw::get_generic_value(unit, ValueId::MadcapStacks) / MadcapPassiveDecayTime);
		if (stacks > 0) {
			//scbw::print(std::to_string(stacks) + ", " + std::to_string(scbw::get_generic_value(unit, ValueId::MadcapStacks)));
			double newCooldown = (double)cooldown * (1.0 - stacks * MadcapAttackSpeedPerStack);
			cooldown = newCooldown;
		}
	}

	// Kaiser Rampage upgrade: gains double attack speed for 1 second on kill
	if (unit->id == UnitId::ZergUltrakor) {
		if (scbw::get_aise_value(unit, NULL, AiseId::GenericTimer, ValueId::KaiserRampage, 0) > 0) {
			cooldown /= 2;
		}
	}

	// Phobos behavior
	if (unit->id == UnitId::TerranPhobos) {
		if (scbw::get_generic_value(unit, ValueId::PhobosDischarge) > 0) {
			cooldown /= 2;
		}
	}

	// Atreus Stalwart Stance behavior: gains 125% attack speed while shields remain intact
	if (unit->id == UnitId::ProtossAtreus
		&& unit->getCurrentShieldsInGame() > 1) { // Leaving it at 1 so incidental regen doesn't give the bonus
		cooldown -= cooldown / 4;
	}

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
