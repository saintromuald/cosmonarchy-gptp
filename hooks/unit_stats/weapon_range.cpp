//Contains hooks that control attack range and seek range (AKA target acquisition range).

#include "weapon_range.h"
#include <SCBW/api.h>

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
			if (scbw::get_generic_value(unit, ValueId::ExemplarSwitch)) {
				bonusAmount += 6;
			}
		}
		case UnitId::ZergBasilisk:
		{
			ActiveTile tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
			if (tile.hasCreep || tile.creepReceeding) {
				bonusAmount += 2;
			}
			break;
		}
		case UnitId::TerranMadrigal: {
			if (scbw::get_generic_value(unit, ValueId::TempoChange)) {
				bonusAmount += 4;
			}
		}
		default:
			bonusAmount += 0;
	}

	// Robotic Sapience - Barghest passive
	if (scbw::get_generic_value(unit, ValueId::RoboticSapience)) {
		u32 stacks = scbw::get_generic_value(unit, ValueId::RoboticSapience);
		if (stacks >= 10) {
			bonusAmount += 5;
		}
		else {
			bonusAmount += stacks * 0.5;
		}
	}

	if (weapons_dat::MaxRange[unit->getGroundWeapon()] <= 32) {
		bonusAmount = 0;
	}

	seekRange = units_dat::SeekRange[unitId] + bonusAmount;

	if (scbw::canUseCliffAdventage(unit)) {
		seekRange += 2;
		seekRange -= (2 - std::max(scbw::getCliffAdvantage(unit), 0)); // ideally we would pass weapon id
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
		case UnitId::ZergBasilisk:
			ActiveTile tile = scbw::getActiveTileAt(unit->position.x, unit->position.y);
			if (tile.hasCreep || tile.creepReceeding) {
				bonusAmount += 64;
			}
			break;
		case UnitId::ProtossExemplar:
		{
			if (scbw::get_generic_value(unit, ValueId::ExemplarSwitch)) {
				bonusAmount += 6*32;
			}
		}
		case UnitId::TerranMadrigal: {
			if (scbw::get_generic_value(unit, ValueId::TempoChange)) {
				bonusAmount += 4*32;
			}
		}
		default:
			break;
	}

	// Robotic Sapience - Barghest passive
	if (scbw::get_generic_value(unit, ValueId::RoboticSapience)) {
		u32 stacks = scbw::get_generic_value(unit, ValueId::RoboticSapience);
		if (stacks >= 10) {
			bonusAmount += 5 * 32;
		}
		else {
			bonusAmount += (stacks * 16);
		}
	}

	if (scbw::canUseCliffAdventage(unit, weaponId)) {
		bonusAmount += 128;
		bonusAmount -= (2 - std::max(scbw::getCliffAdvantage(unit), 0)) * 64;
	}

	if (weapons_dat::MaxRange[unit->getGroundWeapon()] <= 32) {
		bonusAmount = 0;
	}

	return weapons_dat::MaxRange[weaponId] + bonusAmount;

}

} //hooks
