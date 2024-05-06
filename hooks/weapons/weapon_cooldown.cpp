#include "weapon_cooldown.h"
#include "../../BW/bwdata.h"
#include "../../BW/enumerations.h"
#include "../../BW/api.h"
#include "../../math_utils.h"
#include "globals.h"

namespace hooks {
	u32 get_modified_armor_pen(Unit* unit, Unit* target, u8 weapon_id) {
		u32 armor_pen = weapons_dat::armor_pen[weapon_id];
		if (!unit) {
			return armor_pen;
		}

		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueTriggerArmorPenBaseSet]) {
			armor_pen = unit_ext->values[ValueTriggerArmorPenBaseAmount];
		}

		if (unit_ext->unit_vectors[UnitVectorBarghests].size()) {
			armor_pen += unit_ext->unit_vectors[UnitVectorBarghests].size();
		}

		switch (unit->id) {
			case UnitId::TerranOlympian: {
				// Olympian Dishonorable Discharge - add 3 armor pen to targets facing away from the Olympian
				// Audiovisuals should be handled in weapon_damage eventually to account for behaviors that make attacks miss, redirect, etc
				if (target && !units_dat::group_flags[target->id].is_building) {
					if (bw::unit_is_exposed_to_position(target, unit->position)) {
						UnitExt* target_ext = get_unit_ext(target);
						if (!target_ext->timers[TimerOlympianMythicalMightSFX]) {
							helper::play_sound_from_unit(SoundId::OlympianMythicalMight01, target, 1, 0);
							target_ext->timers[TimerOlympianMythicalMightSFX] = 3;
						}
						u32 overlay_id = bw::rand_between(SpriteId::OlympianDishonorableDischarge1, SpriteId::OlympianDishonorableDischarge3);
						bw::create_and_init_thingy(overlay_id, target->position.x, target->position.y, unit->player, target->sprite->elevation + 1);
						armor_pen += 3;
					}
				}
			} break;
		}

		return armor_pen;
	}

	u32 get_modified_cooldown(Unit* unit, u32 cooldown_param, bool clamp) {
		UnitExt* unit_ext = get_unit_ext(unit);
		f32 cooldown = cooldown_param;

		// Stimpack
		if (unit->stim_timer) {
			cooldown /= 2.5;
		}

		// Ensnare
		if (unit->ensnare_timer) {
			switch (unit_ext->values[ValueEnsnareFactor])
			{
				case 1:
				{
					cooldown += cooldown / 3;
				} break;
				case 2:
				{
					cooldown += cooldown / 1.5;
				} break;
			}
		}

		// Crucible Aura
		bool has_crucible = unit_ext->unit_vectors[UnitVectorCrucibles].size() != 0;
		cooldown /= 1 + (has_crucible * 0.23); // Divisor math is different

		// Vithrilisk - Corrosive Spores: lower attack speed of victim
		cooldown *= 1 + unit_ext->values[ValueCorrosiveTouchStacks] * VithriliskCorrosiveTouchCooldownIncrease;

		// Protoss Cornerstone: Ascend - attack cooldown
		if (unit_ext->timers[TimerAscend]) {
			cooldown /= 1.25;
		}

		switch (unit->id) {
			case UnitId::TerranMadcap: {
				// Madcap Cabeirus Feeders behavior: increase weapon speed while stationary
				u32 stacks = ceil((f64)unit_ext->values[ValueMadcapStacks] / MadcapPassiveDecayTime);
				if (stacks > 0) {
					f64 new_cooldown = (f64)cooldown / (1 + stacks * 0.15);
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
			case UnitId::ZergGhitorokor:
			{
				// Relentless Aggression
				if (unit_ext->values[ValueGhitorokorRelentlessAggressionStacks])
				{
					cooldown /= 1 + (0.10 * unit_ext->values[ValueGhitorokorRelentlessAggressionStacks]);
				}
			} break;
			case UnitId::ZergMortothrokor:
			{
				// Whetted Appetite
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

	f32 get_weapon_cooldown_float(Unit* unit, u8 weapon_id) {
		return get_modified_weapon_cooldown(unit, weapon_id) / 24.0f;
	}

	f32 get_current_dps(Unit* unit, u8 weapon_id) {
		return weapons_dat::damage_amount[weapon_id] / get_weapon_cooldown_float(unit, weapon_id);
	}

	f32 get_current_dps_from_damage(Unit* unit, u8 weapon_id, u32 damage) {
		return damage / get_weapon_cooldown_float(unit, weapon_id);
	}
}