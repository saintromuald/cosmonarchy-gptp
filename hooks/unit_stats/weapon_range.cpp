#include "weapon_range.h"
#include "hooks/cosmonarchy_finder.h"
#include <SCBW/api.h>
#include "globals.h"
namespace hooks
{
	// Returns the modified seek distance (AKA target acquisition distance) for the unit.
	// This hook affects the behavior of Unit::get_seek_range().
	// Equivalent to GetTargetAckRangeUpgrade @ 0x00476000  
	u8 get_seek_range(Unit* unit, Unit* target) { // seek distance is measured in tiles, unfortunately
		UnitExt* unit_ext = get_unit_ext(unit);
		u8 bonus_amount = 0;

		// Madrigal - Tempo Change
		switch (unit->id) {
			case UnitId::TerranMadrigalImperiosoMode: {
				if (unit_ext->values[ValueMadrigalTempoDown]) {	
					bonus_amount += 4;
				}
			} break;
		}

		// Repentant Chorus - Barghest behavior
		bonus_amount += unit_ext->values[ValueRepententChorus] / 4;

		// Stellar Enforcement - Magister behavior
		if (!target || get_unit_ext(target)->timers[TimerStellarEnforcement]) {
			bonus_amount += 2;
		}

		f32 scale = 1;

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			scale += 0.05 * unit_ext->values[ValueSovrolethLinkCount];
		}

		// Height advantage
		bonus_amount += scbw::get_cliff_advantage(unit, target, 0, true);

		// Ignore bonus distance for melee units
		if (weapons_dat::max_range[unit->get_ground_weapon()] <= 32 || scbw::unit_is_trap(unit)) {
			scale = 1;
			bonus_amount = 0;
		}

		// Compile variable for use in height advantage calculations
		return units_dat::seek_range[unit->id] * scale + bonus_amount;
	};

	// Returns the modified max distance for the weapon, which is assumed to be attached to the given unit.
	// This hook affects the behavior of Unit::get_max_weapon_range().
	// Note: Weapon ranges are measured in pixels.
	u32 get_max_weapon_range(Unit* unit, u8 weapon_id, Unit* target) {
		UnitExt* unit_ext = get_unit_ext(unit);
		u32 bonus_amount = 0;

		// Give bonus distance to units inside Bunkers
		if (unit->status & UnitStatus::InBuilding) {
			bonus_amount += 64;
		}

		// Madrigal - Tempo Change
		switch (unit->id) {
			case UnitId::TerranMadrigalImperiosoMode: {
				if (weapon_id == WeaponId::MadrigalFuriaMissiles && unit_ext->values[ValueMadrigalTempoDown]) {
					bonus_amount += 4 * 32;
				}
			} break;
		}

		// Repentent Chorus - Barghest behavior
		if (unit->id == UnitId::ProtossBarghest) {
			bonus_amount += unit_ext->values[ValueRepententChorus] * 8;
		}

		// Stellar Enforcement - Magister behavior
		if (target && weapons_dat::max_range[weapon_id] > 32) {
			UnitExt* target_ext = get_unit_ext(target);
			if (target_ext->timers[TimerStellarEnforcement]) {
				bonus_amount += 64;
			}
		}

		f32 scale = 1;

		// Sovroleth - Writhing Web
		if (unit->id == UnitId::ZergSovroleth) {
			scale += 0.05 * unit_ext->values[ValueSovrolethLinkCount];
		}

		// Height advantage
		bonus_amount += scbw::get_cliff_advantage(unit, target, weapon_id, false) * 32;

		// Ignore bonus distance for melee units
		if (weapons_dat::max_range[unit->get_ground_weapon()] <= 32 || scbw::unit_is_trap(unit)) {
			scale = 1;
			bonus_amount = 0;
		}

		return weapons_dat::max_range[weapon_id] * scale + bonus_amount;
	}
}