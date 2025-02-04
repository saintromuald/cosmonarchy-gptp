#include "ai_common.h"
#include <BW/enumerations.h>
#include <BW/api.h>
#include <BW/UnitFinder.h>
#include <globals.h>
#include <cassert>

namespace AI {
	bool is_target_worth_hitting(Unit* target, Unit* attacker) { // logically equivalent to 00440E30
		// If the target is hidden by the fog-of-war
		if (!target->sprite->is_visible_to(attacker->player)) {
			return false;
		}
		// If the target is not detectable
		if (target->status & 0x300 && !((1 << attacker->player) & target->detection_status) && !target->is_visible_to(attacker->player)) {
			return false;
		}
		if (!attacker->is_target_enemy(target)) {
			return false;
		}
		if (unit_id_is_trap(target->id) || target->id == UnitId::ProtossSteward) {
			return false;
		}
		return true;
	}

	bool is_target_attacking_ally(Unit* target, Unit* unit) {
		if (Unit* target_of_target = target->order_target.unit) {
			if (target_of_target->player < 8 && !unit->is_target_enemy(target_of_target)) {
				return true;
			}
		}
		return false;
	}

	bool is_campaign_ai(u8 player) {
		assert(player < PlayerLimit);
		return g_aiscript_controller[player].ai_flags.is_use_map_settings;
	}

	// Internal use only
	const u32 Func_GetRegionIdAtPosEx = 0x0049C9F0;
	u16 get_region_at_position_exact(s32 x, s32 y) {
		static u16 result;
		__asm {
			PUSHAD
			MOV EDI, x
			MOV ECX, y
			CALL Func_GetRegionIdAtPosEx
			MOV result, AX
			POPAD
		}
		return result;
	}

	// Based on code @ 0x00440BB0 (not a real independant function)
	bool unit_is_in_unsafe_region(Unit* unit) {
		u16 current_region = get_region_at_position_exact(unit->get_x(), unit->get_y());
		AiRegion* current_ai_region = &g_ai_region_captains[unit->player][current_region];
		return current_ai_region->state == 3 || current_ai_region->air_target || current_ai_region->ground_target || current_ai_region->flags & 0x20;
	}

	//-------- Get total unit stats in box --------//

	// 00440A60  unit_shields_clump_in_area_accumulator_proc
	// 00440AC0  unitEnergyClumpInAreaAccumulatorProc
	// 00440B20  getEnemyClumpValueProc
	// 00440CF0  AIEnemyOrganicUnitHPAccumulatorProc
	// 00440D60  AIAllyUnitHPAccumulatorProc
	// 00440DC0  AIEnemyUnitHPAccumulatorProc
	// 00440FF0  FindNearestEnemyAndClumpProc

	bw::UnitFinder unitStatTotalFinder;

	// In original functions:
	// 0x00695604 would contains weapon_id
	// 0x00695798 would contains total_enemy_life
	s32 get_total_enemy_life_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster, u8 weapon_id) {
		unitStatTotalFinder.search(x - search_bounds, y - search_bounds, x + search_bounds, y + search_bounds);
		s32 total_enemy_life = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i) {
			Unit* target = unitStatTotalFinder.get_unit(i);
			if ((target != caster) && !(target->status & UnitStatus::Invincible) && bw::can_weapon_target_unit(weapon_id, target, caster) && caster->is_target_enemy(target)) {
				// specific to AIEnemyUnitHPAccumulatorProc @ 0x00440DC0
				if (weapon_id == WeaponId::Plague) {
					total_enemy_life += target->get_current_hp_ingame();
				} else if (weapon_id == WeaponId::MindTyrantMaelstrom) {
					// specific to AIEnemyOrganicUnitHPAccumulatorProc @ 0x00440CF0 - used only by Maelstrom
					if (target->maelstrom_timer == 0) {
						total_enemy_life += target->get_current_life_ingame();
					}
				} else {
					total_enemy_life += target->get_current_life_ingame();
				}
			}
		}
		return total_enemy_life;
	}

	// Loop content based on AIAllyUnitHPAccumulatorProc @ 0x00440D60
	// 0x00695798 would contain total_ally_life
	s32 get_total_ally_life_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster, u8 weapon_id) {
		unitStatTotalFinder.search(x - search_bounds, y - search_bounds, x + search_bounds, y + search_bounds);
		s32 total_ally_life = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i) {
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) && bw::can_weapon_target_unit(weapon_id, target, caster) && !caster->is_target_enemy(target)) {
				total_ally_life += target->get_current_life_ingame();
			}
		}
		return total_ally_life;
	}

	// Loop content based on unit_shields_clump_in_area_accumulator_proc @ 0x00440A60
	// 0x00695798 would contains total_enemy_shields
	s32 get_total_enemy_shields_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster) {
		unitStatTotalFinder.search(x - search_bounds, y - search_bounds, x + search_bounds, y + search_bounds);
		s32 total_enemy_shields = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i) {
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) && caster->is_target_enemy(target) && bw::unit_has_shields(target)) {
				total_enemy_shields += target->get_current_shields_ingame();
			}
		}
		return total_enemy_shields;
	}

	// Loop content based on unitEnergyClumpInAreaAccumulatorProc @ 0x00440AC0
	// 0x00695798 would contains totalEnemyEnergy
	s32 get_total_enemy_energy_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster) {
		unitStatTotalFinder.search(x - search_bounds, y - search_bounds, x + search_bounds, y + search_bounds);
		s32 totalEnemyEnergy = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count(); ++i) {
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) && caster->is_target_enemy(target) && target->is_valid_caster()) {
				totalEnemyEnergy += (&target->energy)[1]; // unitPointer+0xA3 in original code
			}
		}
		return totalEnemyEnergy;
	}

	// Content based on	AI_NukeRequirementsProc @ 0x00441600 and getEnemyClumpValueProc	@ 0x00440B20
	// 0x00695798 would contains total_nuke_target_value
	// Optimized to stop when reaching total_nuke_target_value == 800 since that's what matters
	s32 get_total_enemy_nuke_value_in_area(s32 x, s32 y, s32 search_bounds, Unit* caster) {
		unitStatTotalFinder.search(x - search_bounds, y - search_bounds, x + search_bounds, y + search_bounds);
		s32 total_nuke_target_value = 0;
		for (s32 i = 0; i < unitStatTotalFinder.get_unit_count() && total_nuke_target_value < 800; ++i) {
			Unit* target = unitStatTotalFinder.get_unit(i);
			if (!(target->status & UnitStatus::Invincible) && caster->is_target_enemy(target)) {
				if ((units_dat::base_property[target->id] & UnitProperty::Worker) || (!(target->status & UnitStatus::Reacts))) {
					total_nuke_target_value += target->get_current_life_ingame();
				}

				if (units_dat::base_property[target->id] & UnitProperty::Building)
				{
					if (target->can_detect() || target->id == UnitId::ZergSurkithCesiant) { // lmao
						total_nuke_target_value = 800;
					}
				}
			}
		}
		return total_nuke_target_value;
	}
}