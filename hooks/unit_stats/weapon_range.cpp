#include "weapon_range.h"
#include "hooks/cosmonarchy_finder.h"
#include <SCBW/api.h>
#include "globals.h"
namespace hooks
{
	// Returns the modified seek range (AKA target acquisition range) for the unit.
	// This hook affects the behavior of Unit::getSeekRange().
	// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
	u8 getSeekRangeHook(Unit* unit) // seek range is measured in tiles, unfortunately
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

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
		if (unit->orderTarget.unit)
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
		u32 seekRange = units_dat::SeekRange[unit->id] + bonusAmount;

		// Height advantage
		seekRange += scbw::getCliffAdvantage(unit, 0) * 2;

		// Creep advantage (Disabled)
		//seekRange += scbw::getCreepAdvantage(unit, 0) * 2;

		return seekRange;
	};

	// Returns the modified max range for the weapon, which is assumed to be attached to the given unit.
	// This hook affects the behavior of Unit::getMaxWeaponRange().
	// Note: Weapon ranges are measured in pixels.
	u32 getMaxWeaponRangeHook(Unit* unit, u8 weaponId)
	{
		UnitExt* unitExt = globals.GetUnitExt(unit);

		u32 bonusAmount = 0;
		// Give bonus range to units inside Bunkers
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
		if (unit->orderTarget.unit
			&& weapons_dat::MaxRange[weaponId] > 32)
		{
			UnitExt* orderUnitExt = globals.GetUnitExt(unit->orderTarget.unit);
			if (orderUnitExt->values[ValueStellarEnforcement])
			{
				bonusAmount += 64;
			}
		}

		// Anticthon - Passive
		bonusAmount += weapons_dat::MaxRange[weaponId] * 0.02 * unitExt->values[ValueAnticthonStacks];

		// Height advantage
		bonusAmount += scbw::getCliffAdvantage(unit, weaponId) * 32;

		// Creep advantage (Disabled)
		//bonusAmount += scbw::getCreepAdvantage(unit, weaponId) * 32;

		return weapons_dat::MaxRange[weaponId] + bonusAmount;
	}

	// Hooked to enable target filtering for units with height advantage or range increases. -Pr0nogo
	s32 get_standard_attack_target(Unit* target, Unit* attacker) // 00442DA0
	{
		if (target != attacker)
		{
			if ((1 << attacker->playerId) & target->sprite->visibilityFlags)
			{
				if (helper::unitCanAttackTarget(target, attacker, 1))
				{
					if (attacker->isTargetEnemy(target))
					{
						if ((!min_range[9] || !helper::isTargetWithinMinRange(attacker, min_range[9], target))
							&& helper::isTargetWithinMinRange(attacker, *max_range_in_px, target))
						{
							Unit* attacker_final = attacker;
							if (attacker->subunit && helper::unitIsSubunit(attacker->subunit))
							{
								attacker_final = attacker->subunit;
							}
							if (!(attacker_final->status & UnitStatus::CanTurnAroundToAttack))
							{
								if (!helper::isAngleInAttackAngle(target->sprite->position.y, target->sprite->position.x, attacker_final, units_dat::GroundWeapon[attacker_final->id]))
								{
									return 0;
								}
							}
							if (!helper::is_AI_target_related(attacker_final, target))
							{
								s32 threat_level = helper::get_threat_level(attacker_final, target);
								helper::add_unit_to_possible_targets(threat_level, target);
							}
						}
					}
				}
			}
		}
		return 0;
	}
}