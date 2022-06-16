#include "weapon_range.h"
#include "hooks/cosmonarchy_finder.h"
#include <SCBW/api.h>
#include "globals.h"
namespace hooks
{
	// Returns the modified seek distance (AKA target acquisition distance) for the unit.
	// This hook affects the behavior of Unit::get_seek_range().
	// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
	u8 getSeekRangeHook(Unit* unit, Unit* target) // seek distance is measured in tiles, unfortunately
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		u8 bonusAmount = 0;
		switch (unit->id)
		{
			case UnitId::TerranMadrigal:
			{
				if (scbw::get_generic_rust_value(unit, RustId::ToggleTempoChange))
				{	
					bonusAmount += 4;
				}
			} break;
			default:
			{
				bonusAmount += 0;
			}
		}

		// Repentant Chorus - Barghest Behavior
		bonusAmount += unitExt->values[ValueRepententChorus] / 4;

		// Stellar Enforcement - Magister behavior
		if (unit->order_target.unit)
		{
			UnitExt* orderUnitExt = globals.get_unit_ext(unit->order_target.unit);

			if (orderUnitExt->values[ValueStellarEnforcement])
			{
				bonusAmount += 2;
			}
		}

		// Ignore bonus distance for melee units
		if (weapons_dat::MaxRange[unit->get_ground_weapon()] <= 32)
		{
			bonusAmount = 0;
		}

		f32 scale = 1;
		scale += 0.02 * unitExt->values[ValueAnticthonStacks];

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			EntryArray sovroleths = scbw::get_writhing_web_sovroleths(unit);
			scale +=  0.05 * sovroleths.count;
		}

		// Compile variable for use in height advantage calculations
		u32 seekRange = units_dat::SeekRange[unit->id] * scale + bonusAmount;

		// Height advantage
		seekRange += scbw::get_cliff_advantage(unit, target, 0);

		return seekRange;
	};

	// Returns the modified max distance for the weapon, which is assumed to be attached to the given unit.
	// This hook affects the behavior of Unit::get_max_weapon_range().
	// Note: Weapon ranges are measured in pixels.
	u32 get_max_weapon_range(Unit* unit, u8 weaponId, Unit* target)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);

		u32 bonusAmount = 0;
		// Give bonus distance to units inside Bunkers
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
			} break;
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
		if (unit->order_target.unit
			&& weapons_dat::MaxRange[weaponId] > 32)
		{
			UnitExt* orderUnitExt = globals.get_unit_ext(unit->order_target.unit);
			if (orderUnitExt->values[ValueStellarEnforcement])
			{
				bonusAmount += 64;
			}
		}

		f32 scale = 1;

		// Anticthon - Passive
		scale += 0.02 * unitExt->values[ValueAnticthonStacks];

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			EntryArray sovroleths = scbw::get_writhing_web_sovroleths(unit);
			scale +=  0.05 * sovroleths.count;
		}

		// Height advantage
		bonusAmount += scbw::get_cliff_advantage(unit, target, weaponId) * 32;

		return weapons_dat::MaxRange[weaponId] * scale + bonusAmount;
	}
}