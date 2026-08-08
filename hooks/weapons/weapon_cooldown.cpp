#include "weapon_cooldown.h"
#include "../../BW/bwdata.h"
#include "../../BW/enumerations.h"
#include "../../BW/api.h"
#include "globals.h"

namespace hooks {
	u32 get_modified_armor_pen(Unit* unit, Unit* target, u8 weapon_id) {
		u32 armor_pen = weapons_dat::armor_pen[weapon_id];
		if (!unit) {
			return armor_pen;
		}

		const auto& unit_ext = unit->ext();
		if (unit_ext.values[ValueTriggerArmorPenBaseSet]) {
			armor_pen = unit_ext.values[ValueTriggerArmorPenBaseAmount];
		}

		if (unit_ext.timers[TimerVitalRemainsArmorPen]) {
			armor_pen++;
		}
		armor_pen += unit_ext.unit_vectors[UnitVectorBarghests].size();
		armor_pen += unit_ext.unit_vectors[UnitVectorManifolds].size();
		armor_pen += unit_ext.unit_vectors[UnitVectorScepters].size();

		// @TODO: Extend weapon ids and remove this
		switch (unit->id) {
			case UnitId::TerranCutlass:       armor_pen -= 1; break;
			case UnitId::TerranCutlassTurret: armor_pen -= 1; break;
			case UnitId::ProtossRenesair:	  armor_pen += 1; break;
			case UnitId::ZergZethalor:		  armor_pen -= 3; break;
			case UnitId::AskosiParaclete:     armor_pen += 1; break;
		}

		return armor_pen;
	}

	u32 get_modified_cooldown(Unit* base_unit, u32 cooldown_param, bool clamp) {
		Unit* unit = base_unit->is_subunit() ? base_unit->subunit : base_unit;
		UnitExt& unit_ext = unit->ext();
		fp32 cooldown{cooldown_param};

		// @TODO: Extend weapon ids and remove this
		switch (unit->id) {
			case UnitId::TerranCutlass:				  cooldown /= 1.35fp; break;
			case UnitId::TerranCutlassTurret:		  cooldown /= 1.35fp; break;
			case UnitId::TerranBroadsideTurret:		  cooldown /= 1.06fp; break;
			case UnitId::ProtossCicatus:		      cooldown /= 1.13fp; break;
			case UnitId::ProtossRenesair:		      cooldown *= 1.10fp; break;
			case UnitId::ProtossSimulacrumAstral:     cooldown /= 1.20fp; break;
			case UnitId::AskosiAvatar:				  cooldown *= 3.00fp; break;
			case UnitId::AskosiPhontis:		          cooldown *= 1.33fp; break;
			case UnitId::AskosiZonguard: {
				if (cooldown != 24fp) {
					cooldown *= 1.56fp;
				}
			} break;
			case UnitId::AskosiElgird:		          cooldown *= 1.10fp; break;
			case UnitId::AskosiAlacyne:		          cooldown /= 1.33fp; break;
			case UnitId::AskosiIconoclast:		      cooldown /= 1.13fp; break;
			case UnitId::AskosiParaclete:		      cooldown *= 1.33fp; break;
			case UnitId::ForerunnerFerrum:		      cooldown *= 1.33fp; break;
			case UnitId::ForerunnerChalyb:		      cooldown *= 1.33fp; break;
		}

		// @TODO: Extend weapon ids and remove this
		switch (unit->id) {
			case UnitId::TerranCutlass:				  cooldown /= 1.35fp; break;
			case UnitId::TerranCutlassTurret:		  cooldown /= 1.35fp; break;
			case UnitId::TerranBroadsideTurret:		  cooldown /= 1.06fp; break;
			case UnitId::ProtossCicatus:		      cooldown /= 1.13fp; break;
			case UnitId::ProtossRenesair:		      cooldown *= 1.10fp; break;
			case UnitId::ProtossSimulacrumAstral:     cooldown /= 1.20fp; break;
			case UnitId::AskosiAvatar:				  cooldown *= 3.00fp; break;
			case UnitId::AskosiPhontis:		          cooldown *= 1.33fp; break;
			case UnitId::AskosiIconoclast:		      cooldown /= 1.13fp; break;
		}

		// Stimpack
		if (unit->stim_timer) {
			switch (unit->id) {
				case UnitId::TerranMaverick:        cooldown /= 2.5fp; break;
				case UnitId::TerranMaverickUpstart: cooldown /= 2;   break;
				default:							cooldown /= 2.5fp; break;
			}
		}

		// Ensnare
		if (unit->ensnare_timer) {
			switch (unit_ext.values[ValueEnsnareFactor]) {
				case 1: cooldown += cooldown / 3fp;   break;
				case 2: cooldown += cooldown / 1.5fp; break;
			}
		}

		// Crucible
		bool has_crucible = unit_ext.unit_vectors[UnitVectorCrucibles].size() != 0;
		cooldown /= 1 + (has_crucible * 0.23fp); // Divisor math is different

		// Vithrilisk - Corrosive Spores: lower attack speed of victim
		cooldown *= 1 + unit_ext.values[ValueCorrosiveTouchStacks] * CorrosiveTouch::CooldownIncrease;

		// Protoss Cornerstone: Ascend - attack cooldown
		if (unit_ext.timers[TimerAscend]) {
			cooldown /= 1.33fp;
		}

		if (unit_ext.values[ValueReassemblyActive]) {
			switch (unit->id) {
				case UnitId::AskosiScepter: {
					cooldown /= 1.33fp;
				} break;
				case UnitId::AskosiTirade: {
					cooldown *= 2;
				} break;
			}
		}

		if (unit_ext.timers[TimerRetaliationAssembly]) {
			cooldown /= 2;
		}

		if (unit_ext.timers[TimerNewDominion]) {
			cooldown /= 1.33fp;
		}

		if (unit_ext.values[ValueReassemblyActive]) {
			switch (unit->id) {
				case UnitId::AskosiScepter: {
					cooldown /= 1.33fp;
				} break;
				case UnitId::AskosiTirade: {
					cooldown *= 2;
				} break;
			}
		}

		if (unit_ext.timers[TimerRetaliationAssembly]) {
			cooldown /= 2;
		}

		if (unit_ext.timers[TimerNewDominion]) {
			cooldown /= 1.33fp;
		}

		if (unit_ext.timers[TimerGoremastSlow]) {
			cooldown *= 1.33fp;
		}

		if (unit_ext.values[ValueIsShade]) {
			cooldown /= 1.5fp;
		}

		if (unit_ext.timers[TimerDivineDisciplineBuff]) {
			cooldown /= 1.25fp;
		}
		if (unit_ext.timers[TimerDivineDisciplineDebuff]) {
			cooldown /= 0.75fp;
		}

		switch (unit->id) {
			case UnitId::TerranMadcap: {
				// Madcap Cabeirus Feeders behavior: increase weapon speed while stationary
				auto stacks = ceil((fp32)unit_ext.values[ValueMadcapStacks] / TerranMadcap::PassiveDecayTime);
				if (stacks > 0fp) {
					fp32 new_cooldown = cooldown / (1 + stacks * 0.15fp);
					cooldown = new_cooldown;
				}
			} break;
			case UnitId::TerranBlackjack:
			case UnitId::TerranBlackjackGolden: {
				// Blackjack Wear and Tear: reduce weapon speed based on missing health
				if (unit_ext.values[ValueWearAndTear]) {
					cooldown /= 0.66fp;
				}
			} break;
			case UnitId::TerranMatadorCinderMode: {
				cooldown /= 1.25fp;
			} break;
			case UnitId::TerranPhobos: {
				// Argent Capacitors
				if (unit_ext.values[ValuePhobosDischarge] > 0) {
					cooldown /= 2;
				}
			} break;
			case UnitId::ProtossVagrantShade: {
				cooldown /= 1.5fp;
			} break;
			case UnitId::ZergGhitorokor: {
				// Relentless Aggression
				if (unit_ext.values[ValueGhitorokorRelentlessAggressionStacks]) {
					cooldown /= 1 + (0.10fp * unit_ext.values[ValueGhitorokorRelentlessAggressionStacks]);
				}
			} break;
			case UnitId::ZergMortothrokor: {
				// Whetted Appetite
				if (unit_ext.values[ValueMortothrokorStacks]) {
					cooldown /= 1 + (0.05fp * unit_ext.values[ValueMortothrokorStacks]);
				}
			} break;
			case UnitId::ZergUltrokor: {
				// Kaiser Rampage upgrade: gains double attack speed for 1 second on kill
				if (unit_ext.timers[TimerKaiserRampage] > 0) {
					cooldown /= 2;
				}
			} break;
		}

		// Ion Cannon Ion Field: Attack Speed Buff
		if (unit_ext.timers[TimerIonFieldAttackSpeed] > 0) {
			cooldown /= 1.5fp;
		}

		// Hierophant Signify: Malice
		if (unit_ext.values[ValueSignifyStacks] >= 5) {
			cooldown *= 1.5fp;
		}

		if (unit->stim_timer) { // Fixes aice-related Stimpack issue
			cooldown += 2;
		}

		// Power Siphon
		if (unit_ext.values[ValueSavantPowerSiphonBuff]) {
			cooldown /= 1 + (0.1fp * unit_ext.values[ValueSavantPowerSiphonBuff]);
		}
		if (unit_ext.values[ValueSavantPowerSiphonDebuff]) {
			cooldown *= 1 + (0.1fp * unit_ext.values[ValueSavantPowerSiphonDebuff]);
		}

		if (clamp) {
			cooldown = std::clamp(cooldown, 1fp, 250fp);
		}

		return static_cast<u32>(cooldown);
	}

	/// Calculates the unit's weapon cooldown, factoring in upgrades and status effects.
	/// @return	the modified cooldown value.
	u32 get_modified_weapon_cooldown(Unit* unit, u8 weapon_id) {
		auto cooldown = weapons_dat::cooldown[weapon_id];
		switch (unit->id) {
			case UnitId::AskosiMimic: {
				cooldown /= weapons_dat::damage_factor[weapon_id];
			} break;
			case UnitId::AskosiZonguard: { // TODO: Extend weapons and remove this
				if (weapon_id == WeaponId::GrandstandArteDriver) {
					cooldown = 24;
				}
			}
		}
		return get_modified_cooldown(unit, cooldown, true);
	}

	fp32 get_weapon_cooldown_fp(Unit* unit, u8 weapon_id) {
		return get_modified_weapon_cooldown(unit, weapon_id) / 24.0fp;
	}

	fp32 get_current_dps(Unit* unit, u8 weapon_id) {
		return weapons_dat::damage_amount[weapon_id] / get_weapon_cooldown_fp(unit, weapon_id);
	}

	fp32 get_current_dps_from_damage(Unit* unit, u8 weapon_id, u32 damage) {
		return damage / get_weapon_cooldown_fp(unit, weapon_id);
	}
}