//Contains hooks that control attack range and seek range (AKA target acquisition range).

#include "weapon_range.h"
#include "hooks/cosmonarchy_finder.h"
#include <SCBW/api.h>
#include "globals.h"
namespace hooks
{

	/// Returns the modified seek range (AKA target acquisition range) for the unit.
	/// Note: Seek ranges are measured in matrices (1 matrix = 32 pixels).
	/// This hook affects the behavior of Unit::getSeekRange().
	/// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
	u8 getSeekRangeHook(Unit* unit) // seek range is measured in tiles, unfortunately
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		u16 unitId = unit->id;
		u32 seekRange;
		u8 bonusAmount = 0;

		switch (unitId)
		{
			case UnitId::TerranMadrigal:
			{
				if (scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange))
				{
					bonusAmount += 4;
				}
			}
			default:
			{
				bonusAmount += 0;
			}
		}

		// Repentant Chorus - Barghest Behavior
		bonusAmount += unitExt->values[ValueRepententChorus] / 4;

		// Stellar Enforcement - Magister behavior
		if (unit->orderTarget.unit != nullptr)
		{
			UnitExt* orderUnitExt = globals.GetUnitExt(unit->orderTarget.unit);

			if (orderUnitExt->values[ValueStellarEnforcement])
			{
				bonusAmount += 2;
			}
		}

		// Ignore bonus range for melee units
		if (weapons_dat::MaxRange[unit->getGroundWeapon()] <= 32)
		{
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
	/// This hook affects the behavior of Unit::getMaxWeaponRange().
	/// Note: Weapon ranges are measured in pixels.
	///
	/// @param	weapon		The weapons.dat ID of the weapon.
	/// @param	unit			The unit that owns the weapon. Use this to check upgrades.
	u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		u32 bonusAmount = 0;
		//Give bonus range to units inside Bunkers
		if (unit->status & UnitStatus::InBuilding)
		{
			bonusAmount = 64;
		}

		switch (unit->id)
		{
			case UnitId::TerranMadrigal:
			{
				if (scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange))
				{
					bonusAmount += 4 * 32;
				}
			}
			default:
			{
				break;
			}
		}

		// Repentent Chorus - Barghest behavior
		if (unit->id == UnitId::ProtossBarghest)
		{
			bonusAmount += unitExt->values[ValueRepententChorus] * 8;
		}

		// Stellar Enforcement - Magister behavior
		if (unit->orderTarget.unit != nullptr
			&& weapons_dat::MaxRange[weaponId] > 32)
		{
			UnitExt* orderUnitExt = globals.GetUnitExt(unit->orderTarget.unit);
			if (orderUnitExt->values[ValueStellarEnforcement])
			{
				bonusAmount += 64;
			}
		}

		// Anticthon - Passive
		bonusAmount += weapons_dat::MaxRange[weaponId] * 0.1 * unitExt->values[ValueAnticthonStacks];

		// Height advantage
		bonusAmount += 128;
		bonusAmount -= (2 - std::max(scbw::getCliffAdvantage(unit, weaponId), 0)) * 64;

		// Creep advantage
		bonusAmount += 64;
		bonusAmount -= (1 - std::max(scbw::getCreepAdvantage(unit, weaponId), 0)) * 64;

		return weapons_dat::MaxRange[weaponId] + bonusAmount;
	}

} //hooks
