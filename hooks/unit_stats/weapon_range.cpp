//Contains hooks that control attack range and seek range (AKA target acquisition range).

#include "weapon_range.h"
#include "hooks/hydraFinder.h"
#include <SCBW/api.h>
#include "globals.h"
namespace hooks {

/// Returns the modified seek range (AKA target acquisition range) for the unit.
/// Note: Seek ranges are measured in matrices (1 matrix = 32 pixels).
/// This hook affects the behavior of CUnit::getSeekRange().
/// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
u8 getSeekRangeHook(CUnit* unit) {

	u16 unitId = unit->id;
	u32 seekRange;
	u8 bonusAmount = 0;

	switch (unitId) {
		case UnitId::ProtossExemplar:
		{
			if (scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch)) {
				bonusAmount += 6;
			}
		}
		case UnitId::TerranMadrigal: {
			if (scbw::get_generic_rust_value(unit, ValueId::TempoChange)) {
				bonusAmount += 4;
			}
		}
		default:
			bonusAmount += 0;
	}

	// Robotic Sapience - Barghest behavior
/*	if (scbw::get_generic_value(unit, ValueId::RoboticSapience)) {
		u32 stacks = scbw::get_generic_value(unit, ValueId::RoboticSapience);
		if (stacks >= 10) {
			bonusAmount += 5;
		}
		else {
			bonusAmount += stacks * 0.5;
		}
	}*/
	
	//Repentent Chorus - Barghest Behavior
	auto barghestCount = hydraUnitCount(unit, unit->position, 5 * 32, CFlags::Ally | CFlags::NotSelf, { UnitId::ProtossBarghest });
	bonusAmount += barghestCount / 4;//seek range is measured in tiles, unfortunately
	/*
	int r = 16;
					auto n = unitRangeCount(nullptr, CFlags::NoInvincible | CFlags::Enemy | CFlags::NoAir, {});
					scbw::UnitFinder unitsInSplash(impact.position.x - r, impact.position.y - r,
						impact.position.x + r, impact.position.y + r);
	*/

	// Stellar Enforcement - Magister behavior
	if (unit->orderTarget.unit != nullptr
	&& scbw::get_generic_value(unit->orderTarget.unit, ValueId::StellarEnforcement)) {
		bonusAmount += 2;
	}

	// Ignore bonus range for melee units
	if (weapons_dat::MaxRange[unit->getGroundWeapon()] <= 32) {
		bonusAmount = 0;
	}
	
	// Compile variable for use in height advantage calculations
	seekRange = units_dat::SeekRange[unitId] + bonusAmount;

	if (unit->orderTarget.unit != nullptr
	&& !(units_dat::BaseProperty[unit->orderTarget.unit->id] & UnitProperty::Flyer))
	{
		// Height advantage
		seekRange += 2;
		seekRange -= (2 - std::max(scbw::getCliffAdvantage(unit), 0));

		// Creep advantage
		seekRange += 1;
		seekRange -= (1 - std::max(scbw::getCreepAdvantage(unit), 0));
	}

	return seekRange;
};

/// Returns the modified max range for the weapon, which is assumed to be
/// attached to the given unit.
/// This hook affects the behavior of CUnit::getMaxWeaponRange().
/// Note: Weapon ranges are measured in pixels.
///
/// @param	weapon		The weapons.dat ID of the weapon.
/// @param	unit			The unit that owns the weapon. Use this to check upgrades.
u32 getMaxWeaponRangeHook(CUnit* unit, u8 weaponId) {
	//Default StarCraft behavior

	u32 bonusAmount = 0;


	//Give bonus range to units inside Bunkers
	if (unit->status & UnitStatus::InBuilding) {
		bonusAmount = 64;
	}

	switch (unit->id) {
		case UnitId::ProtossExemplar:
		{
			if (scbw::get_generic_rust_value(unit, ValueId::ExemplarSwitch)) {
				bonusAmount += 6*32;
			}
		}
		case UnitId::TerranMadrigal: {
			if (scbw::get_generic_rust_value(unit, ValueId::TempoChange)) {
				bonusAmount += 4*32;
			}
		}
		default:
			break;
	}

	// Repentent Chorus - Barghest behavior
	if (unit->id == UnitId::ProtossBarghest) {
		int r = 5 * 32;
		auto barghestCount = hydraUnitCount(unit, unit->position, 5 * 32, CFlags::Ally | CFlags::NotSelf, { UnitId::ProtossBarghest });
		bonusAmount += barghestCount * 8;
	}

	// Stellar Enforcement - Magister behavior
	if (unit->orderTarget.unit != nullptr
	&&	scbw::get_generic_value(unit->orderTarget.unit, ValueId::StellarEnforcement)) {
		bonusAmount += 64;
	}

	// Height advantage
	bonusAmount += 128;
	bonusAmount -= (2 - std::max(scbw::getCliffAdvantage(unit, weaponId), 0)) * 64;

	// Creep advantage
	bonusAmount += 64;
	bonusAmount -= (1 - std::max(scbw::getCreepAdvantage(unit, weaponId), 0)) * 64;

	return weapons_dat::MaxRange[weaponId] + bonusAmount;
}

} //hooks
