#include "weapon_cooldown.h"
#include "../../SCBW/scbwdata.h"
#include "../../SCBW/enumerations.h"
#include "../../SCBW/api.h"
#include "../../math_utils.h"
#include "globals.h"

namespace hooks {
	u32 get_modified_armor_pen(Unit* unit, Unit* target, u8 weapon_id) {
		u32 armor_pen = weapons_dat::armor_pen[weapon_id];
		if (!unit) {
			return armor_pen;
		}

		switch (unit->id) {
			case UnitId::TerranOlympian: {
				// Olympian Dishonorable Discharge - add 3 armor pen to targets facing away from the Olympian
				if (target && !units_dat::group_flags[target->id].is_building) {
					if (scbw::unit_is_exposed_to_position(target, unit->position)) {
						helper::play_sound_from_unit(SoundId::OlympianMythicalMight01, target, 1, 0);
						// TODO: Overlay
						armor_pen += 3;
					}
				}
			} break;
		}

		return armor_pen;
	}

	u32 get_modified_cooldown(Unit* unit, u32 cooldown_param, bool clamp) {
		UnitExt* unit_ext = get_unit_ext(unit);

		// Set modifier based on Stim (increase), Ensnare (decrease), and attackspeed upgrades (increase)
		s32 cooldown_modifier = (unit->stim_timer ? 1 : 0) - (unit->ensnare_timer ? 1 : 0)
			+ (unit->status & UnitStatus::CooldownUpgrade ? 1 : 0);

		f32 cooldown = cooldown_param;

		// Crucible Aura
		cooldown /= 1 + 0.25 * unit_ext->unit_vectors[UnitVectorCrucibles].size();

		// Vithrilisk - Corrosive Spores: lower attack speed of victim
		cooldown *= 1 + unit_ext->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchCooldownIncrease;

		// Vassal Terminal Surge
		if (unit_ext->timers[TimerTerminalSurge] > 1) {
			cooldown /= 1.25;
		}

		switch (unit->id) {
			case UnitId::TerranMadcap: {
				// Madcap Cabeirus Feeders behavior: increase weapon speed while stationary
				u32 stacks = ceil((f64)unit_ext->values[ValueMadcapStacks] / MadcapPassiveDecayTime);
				if (stacks > 0) {
					f64 new_cooldown = (f64)cooldown * (1.0 - stacks * MadcapAttackSpeedPerStack);
					cooldown = new_cooldown;
				}
			} break;
			case UnitId::TerranBlackjack: {
				// Blackjack Wear and Tear: reduce weapon speed based on missing health
				const u32 health = unit->hitpoints / 256;
				const u32 max_health = unit->get_max_hp_ingame();
				const f64 multiplier = MathUtils::map(health, (u32)0, max_health, 0.66, 1.0, true);
				cooldown /= multiplier;
			} break;
			case UnitId::TerranPhobos: {
				// Argent Capacitors
				if (unit_ext->values[ValuePhobosDischarge] > 0) {
					cooldown /= 2;
				}
			} break;
			case UnitId::ProtossVagrantShade: {
				cooldown /= 1.5;
			} break;
			case UnitId::ZergMortothrokor:
			{
				if (unit_ext->values[ValueMortothrokorStacks])
				{
					cooldown /= 1 + (0.05 * unit_ext->values[ValueMortothrokorStacks]);
				}
			} break;
			case UnitId::ZergUltrokor: {
				// Kaiser Rampage upgrade: gains double attack speed for 1 second on kill
				if (unit_ext->timers[TimerKaiserRampage] > 0) {
					cooldown /= 2;
				}
			} break;
		}

		// Ion Cannon Ion Field: Attack Speed Buff
		if (unit_ext->timers[TimerIonFieldAttackSpeed] > 0) {
			cooldown /= 1.5;
		}

		// Hierophant Signify: Malice
		if (unit_ext->values[ValueSignifyStacks] >= 5) {
			cooldown /= 0.6666;
		}

		if (cooldown_modifier > 0) { // Lame hardcode for attackspeed upgrades
			cooldown /= 2;
		} else {
			if (cooldown_modifier < 0) { // Presumably, lame hardcode for some slow
				cooldown += cooldown / 4;
			}
		}

		if (unit->stim_timer) { // Fixes aice-related Stimpack issue
			cooldown += 2;
		}

		// Power Siphon
		if (unit_ext->values[ValueSavantPowerSiphonBuff]) {
			cooldown /= 1 + (0.1 * unit_ext->values[ValueSavantPowerSiphonBuff]);
		}
		if (unit_ext->values[ValueSavantPowerSiphonDebuff]) {
			cooldown *= 1 + (0.1 * unit_ext->values[ValueSavantPowerSiphonDebuff]);
		}

		if (clamp) {
			if (cooldown > 250) { // Max cutoff
				cooldown = 250;
			} else if (cooldown < 1) { // Min cutoff (previously 5)
				cooldown = 1;
			}
		}

		return cooldown;
	}

	/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
	/// @return	the modified cooldown value.
	u32 get_modified_weapon_cooldown(Unit* unit, u8 weapon_id) {
		return get_modified_cooldown(unit, weapons_dat::cooldown[weapon_id], true);
	}

	f32 get_weapon_cooldwon_float(Unit* unit, u8 weapon_id) {
		return get_modified_weapon_cooldown(unit, weapon_id) / 24.0f;
	}

	f32 get_current_dps(Unit* unit, u8 weapon_id) {
		return weapons_dat::damage_amount[weapon_id] / get_weapon_cooldwon_float(unit, weapon_id);
	}

	f32 get_current_dps_from_damage(Unit* unit, u8 weapon_id, u32 damage) {
		return damage / get_weapon_cooldwon_float(unit, weapon_id);
	}
}