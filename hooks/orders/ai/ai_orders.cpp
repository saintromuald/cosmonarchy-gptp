#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"
#include "ai_layouts.h"
#include "globals.h"
#include "custom_ai.h"
#include "hooks/harvest.h"

static s32 compare_gas_buildings(const void* a, const void* b) {
	Unit* ua = *(Unit**)a;
	Unit* ub = *(Unit**)b;
	return get_harvest_amount(ub) - get_harvest_amount(ua);
}

void sort_gas_buildings(AiTown* town) {
	AiTownExt* town_ext = get_town_ext(town);
	qsort(town_ext->gas_buildings, town_ext->gas_buildings_count, sizeof(Unit*), compare_gas_buildings);
}

namespace hooks {
	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
	// TODO: finish and cleanup
	void ai_unit_was_hit(Unit* unit, Unit* attacker, s32 main_target_reactions, s32 dont_call_help) { // 0043F320
		if (g_player_table[unit->player].type != PlayerType::Computer || unit->player >= PlayerLimit) {
			return;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Building && unit->unit_ai) {
			g_aiscript_controller[unit->player].previous_building_hit_second = *g_elapsed_time_seconds;
			if (unit->unit_ai->type == AiType::Building) {
				unit->unit_ai->building_ai.town->in_battle = true;
			}
			if (main_target_reactions && attacker) {
				// Cancel structure code
				if (!(units_dat::base_property[unit->id] & UnitProperty::Worker) 
					&& unit->main_order_id // Original function call checks if unit has a sprite??? 00475A50
					&& !(unit->status & UnitStatus::Completed)) {
					u32 threshold = std::max(10, unit->get_total_hp_plus_shields() / 16); // Used to be divided by 4 -Veeq7
					if (unit->get_current_life_ingame() < threshold) {
						helper::cancel_unit(unit);
						return;
					}
				}
			}
		}

		Unit* target = helper::ai_get_actual_target(unit, attacker);
		if (target->status & UnitStatus::InBuilding) {
			target = target->connected_unit;
			if (!target) {
				return;
			}
		}
		if (target->status & UnitStatus::CloakingForFree) {
			if (!(target->status & UnitStatus::Burrowed)) {
				target = helper::check_arbiter_cloaking_range(unit, UnitId::ProtossDidact);
				if (!target) {
					return;
				}
			}
		}

		if (main_target_reactions) {
			helper::ai_order_unit_cloaking(unit);
			helper::ai_order_lurker_burrow(unit);
		}

		UnitAi* unit_ai = unit->unit_ai;
		bool ignore_defense = false;
		bool in_attack_group = false;
		bool skip_checks = false; 
		if (unit_ai) {
			if (unit_ai->type == AiType::Military) {
				MilitaryAi* unit_military_ai = (MilitaryAi*)unit_ai;
				if (unit_military_ai->region->state == RegionState::ContinueAttackHere && unit->is_unit_ai_non_building()) {
					helper::ai_set_region_state(RegionState::Attacking, unit_military_ai->region);
				}
				in_attack_group = helper::unit_is_in_attack_group(unit, 0);
			}

			// This logic is a bit weird, but it should be logically 1 to 1 with IDA code, possibly a Lard bug tbh -Veeq7
			if (main_target_reactions && helper::unit_is_active_transport(unit) && unit->status & UnitStatus::Reacts) {
				if (helper::is_unit_loaded(unit)) {
					if (unit->main_order_id != OrderId::Unload) {
						helper::issue_order_targeting_nothing(unit, OrderId::Unload);
					}
					return;
				}
			} else {
				skip_checks = true;
			}
		} else {
			skip_checks = true;
		}

		if (skip_checks || helper::ai_try_return_home(unit, 0) || !helper::ai_return_to_nearest_base_forced(unit)) {
			if (!skip_checks && g_aiscript_controller[unit->player].ai_flags.is_use_map_settings) {
				ignore_defense = true;
			}
			AiRegion* unit_region = scbw::get_ai_region(unit->player, scbw::get_region_id(unit->position));
			if (!ignore_defense) {
				if (!dont_call_help) {
					helper::ai_ask_for_help(unit, target, in_attack_group);
				}
				if (!in_attack_group) {
					AiRegion* target_region = scbw::get_ai_region(unit->player, scbw::get_region_id(target->position));
					if (unit->unit_ai
						&& !helper::unit_is_in_attack_group(unit, 0)
						&& (unit->unit_ai->type != AiType::Guard || !g_aiscript_controller[unit->player].ai_flags.is_use_map_settings)) {
						if (unit_region->state == RegionState::TownRegionHigh || unit_region->state == RegionState::TownRegionLow) {
							helper::ai_set_region_state(RegionState::Defending, unit_region);
						}
						if (unit_region->state && !target_region->state && *g_elapsed_time_seconds > 60) {
							helper::ai_set_region_state(RegionState::TownRegionHigh, target_region);
						}
					}
					if (!dont_call_help) {
						target_region->air_strength_enemy = helper::get_enemy_air_strength_in_region(target_region->region, unit->player);
						target_region->ground_strength_enemy = helper::get_enemy_strength_in_region(target_region->region, unit->player, 0);
						if (unit_region->ground_strength_needed < target_region->ground_strength_needed) {
							unit_region->ground_strength_needed = target_region->ground_strength_needed;
						}
						if (unit_region->air_strength_needed < target_region->air_strength_needed) {
							unit_region->air_strength_needed = target_region->air_strength_needed;
						}
					}
				}
			}

			unit_region->flags |= 0x20u;
			if (unit->status & UnitStatus::Completed) {
				u32 unit_order_id = unit->main_order_id; // Keeping a copy of the order id, since some of the calls later could probably change it -Veeq7
				if (unit_order_id == OrderId::CompletingArchonSummon ||
					unit_order_id == OrderId::ResetCollision1 ||
					unit_order_id == OrderId::ConstructingBuilding) {
					return;
				}

				if (!main_target_reactions && scbw::order_is_spell(unit_order_id)) {
					return;
				}

				if (unit_order_id == OrderId::EnterTransport) {
					return;
				}

				bool is_unbreakable = helper::unit_is_performing_unbreakable_code(unit);
				if (!is_unbreakable && helper::ai_cast_spell_behavior(unit, main_target_reactions)) {
					return;
				}
				
				if (unit_order_id == OrderId::RechargeShields1 ||
					unit_order_id == OrderId::Move) {
					return;
				}

				if (main_target_reactions) {
					if (unit->id == UnitId::ProtossSolarion
						&& unit->unit_with_hangar.in_hangar_count + unit->unit_with_hangar.out_hangar_count <= 2
						&& helper::ai_return_to_nearest_base_forced(unit)) {
						return;
					}

					if (!target || (!(target->status & UnitStatus::Reacts)
						&& !g_aiscript_controller[unit->player].ai_flags.is_use_map_settings)) {
						return;
					}

					// Attempt to return to base when below an HP threshold
					u32 current = unit->get_current_life_ingame();
					u32 total = unit->get_total_hp_plus_shields();

					// Disabled for now... -Veeq7
					/*
					if (current <= total * 3 / 4
						&& scbw::ai_unit_id_can_flee_three_quarters(unit->id)
						&& helper::ai_return_to_nearest_base_forced(unit)) {
						return;
					}

					if (current <= total * 2 / 3
						&& scbw::ai_unit_id_can_flee_two_thirds(unit->id)
						&& helper::ai_return_to_nearest_base_forced(unit)) {
						return;
					}
					*/

					if  (unit->id == UnitId::ZergLakizilisk
					&& !(unit->status & UnitStatus::Burrowed)
					&&   unit_order_id != OrderId::Burrow
					&&   unit_order_id != OrderId::Unburrow)
					{
						if (helper::ai_flee(unit, target)) {
							return;
						}
					}

					if (!is_unbreakable
					&&   unit->status & UnitStatus::Reacts
					&&  !helper::unit_has_ground_path(unit, target->position.x, target->position.y)
					&&   helper::ai_flee(unit, target))
					{
						return;
					}
				}

				if (unit->order_target.unit == target) {
					return;
				}

				// Flee with workers, unless they have to defend (disabled, since kiting is now responsible for this) -Veeq7
				if (units_dat::base_property[unit->id] & UnitProperty::Worker
					/* && !(units_dat::base_property[target->id] & UnitProperty::Worker)
					&& !scbw::unit_is_ranged_attacker(target) // previously only checked for Zethrokor id
					&& !helper::ai_has_no_defense_build(unit->id)*/) {
					if (main_target_reactions) {
						helper::ai_flee(unit, target);
					}
					return;
				}

				// Ignore allied units (new) -Veeq7		
				if (target && scbw::is_allied_to(unit->player, target->player)) {
					UnitExt* unit_ext = get_unit_ext(unit);
					u32 patience = unit_ext->timers[TimerAI_Patience];
					unit_ext->timers[TimerAI_Patience] += 2 * 24;
					if (patience <= 7 * 24) {
						if (units_dat::base_property[unit->id] & UnitProperty::Worker ||
							!helper::unit_can_attack_target(target, unit, 1)) {
							helper::ai_flee(unit, target); // Flee
						}
						return;
					}
				}

				if (main_target_reactions && unit->unit_ai) {
					if (helper::unit_can_see_cloaked_target(target, unit)) {
						helper::ai_respond_to_cloaked_threat(unit, target);
					}
				}

				if (is_unbreakable) {
					return;
				}

				if (helper::unit_can_attack_target(target, unit, 1)) {
					unit->auto_target_unit = target;
					if (unit->subunit->is_subunit()) {
						unit->subunit->auto_target_unit = target;
					}
					helper::ai_update_attack_target(unit, 0, 1, 0);
				} else if (unit->unit_ai) {
					Target flee_target = helper::ai_prepare_to_flee(target, unit);
					if (unit_order_id != OrderId::AIPatrol || flee_target.pt == unit->position) {
						if (main_target_reactions) {
							if (unit->id != UnitId::TerranCleric) {
								if (!unit->order_target.unit || !helper::unit_can_attack_target(unit->order_target.unit, unit->order_target.unit, 1)) {
									helper::ai_flee(unit, target);
								}
							}
						}
					} else {
						helper::order_interrupt(unit, flee_target.unit, flee_target.pt.x, flee_target.pt.y, OrderId::Move);
					}
				}
			}
		}
	}

	// Hooked to help with AI recloaking units after the cloak rewrite - Pr0nogo
	void ai_order_unit_cloaking(Unit* unit) { // 0043B970
		return;

		/*
		s32 unit_status = unit->status;
		if (!(unit_status & 0x100)) {
			s8 tech_id;
			if (unit->id == UnitId::TerranEidolon) {
				tech_id = TechId::PersonnelCloaking;
			} else if (unit->id == UnitId::TerranWraith) {
				tech_id = TechId::CloakingField;
			}
			if (helper::tech_use_allowed(unit, tech_id, unit->player) == 1) {
				helper::apply_cloaking_order(unit);
			}
		}
		*/
	}

	// Hooked to improve functionality - Pr0nogo
	void ai_send_suicide(bool is_random_suicide, s32 player) { // 0043E050
		if (is_random_suicide) {
			for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
				if (unit->sprite) {
					if (unit->main_order_id) {
						if (!unit->is_subunit()
							&& unit->id != UnitId::TerranNuclearMissile
							&& unit->id != UnitId::ZergLarva
							&& unit->id != UnitId::ZergOcsalLarva
							&& !(units_dat::base_property[unit->id] & UnitProperty::Worker)
							&& !(unit->sprite->flags & SpriteFlags::Hidden)
							&& !helper::is_threat(0, unit)) {
							helper::prepare_suicide(unit);
						}
					}
				}
			}
			return;
		}
		if (!g_aiscript_controller[player].strategic_suicide) {
			s32 region_index = 2500;
			AiRegion* ai_region = 0;
			for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
				if (!unit->sprite || !unit->main_order_id) {
					continue;
				}

				UnitAi* unit_ai = unit->unit_ai;
				if (!unit_ai || helper::unit_is_active_transport(unit) || unit->status & UnitStatus::InBuilding
				&& !(g_aiscript_controller[player].ai_flags.is_use_map_settings))
				{
					continue;
				}

				s16 region_id = helper::get_region_id(unit->position.x, unit->position.y);
				AiRegion* region = &g_ai_region_captains[player][region_id];
				if (unit_ai->type == AiType::Guard) {
					GuardAi* guard_ai = &unit->unit_ai->guard_ai;
					guard_ai->parent->unit_ai = 0;
					unit->unit_ai = 0;
					helper::add_military_ai(region, unit, 1);
				} else if (unit_ai->type != AiType::Military) {
					continue;
				}

				if (unit_ai && unit_ai->type == AiType::Military) {
					MilitaryAi* military_ai = &unit->unit_ai->military_ai;
					u8 region_state = military_ai->region->state;
					if (region_state == RegionState::ContinueAttackHere || region_state == RegionState::Attacking || region_state == RegionState::PreparingAttack || region_state == RegionState::PreparingAttackOldUnreachable) {
						continue;
					}
				}

				if (region_index != 2500) {
					helper::remove_unit_ai(unit, false);
					helper::add_military_ai(ai_region, unit, 1);
					continue;
				}

				memset(g_aiscript_controller[player].attackers, 0, sizeof(g_aiscript_controller[player].attackers));
				g_aiscript_controller[player].ai_attack_group_region = 0;
				helper::ai_attack_clear(player);
				if (!helper::ai_attack_manager(player, unit->position.x, unit->position.y, 0, 1)) {
					g_aiscript_controller[player].strategic_suicide = 24;
					region_index = (u16)g_aiscript_controller[player].ai_attack_group_region - 1;
					ai_region = &g_ai_region_captains[player][region_index];
					helper::remove_unit_ai(unit, false);
					helper::add_military_ai(ai_region, unit, 1);
				}
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	void ai_recalculate_region_strength(AiRegion* ai_region) { // 0043A390
		ai_region->ground_strength = 0;
		ai_region->ground_strength_full = 0;
		ai_region->air_strength = 0;
		ai_region->air_strength_full = 0;
		
		// Count own and allied AI force
		for (u32 player = 0; player < PlayerLimit; ++player) {
			if (scbw::is_allied_to(ai_region->player, player)) {
				AiRegion* player_ai_region = scbw::get_ai_region(player, ai_region->region);
				for (MilitaryAi* military_ai = player_ai_region->military.first; military_ai; military_ai = military_ai->link.prev) {
					Unit* unit = (Unit*)military_ai->unit;
					if (!unit || unit->status & UnitStatus::InBuilding) {
						continue;
					}
					ai_region->ground_strength_full += unit->ground_strength;
					ai_region->air_strength_full += unit->air_strength;
					if (helper::get_region_id(unit->position.x, unit->position.y) == ai_region->region) {
						ai_region->ground_strength += unit->ground_strength;
						ai_region->air_strength += unit->air_strength;
					}
				}
			}
		}
		
		// Also count in allied non-AI force
		{
			EntryArray region_units = {};
			unit_search_rect(&region_units, scbw::get_region_area(scbw::get_region(ai_region->region)));
			unit_search_filter(&region_units, 0, FilterCanAttackOrCast | FilterNotWorker | FilterNoTrap);
			for (u32 i = 0; i < region_units.count; ++i) {
				Unit* unit = region_units.units[i];
				if (!scbw::is_allied_to(ai_region->player, unit->player)) {
					continue;
				}
				if (unit->unit_ai && unit->unit_ai->type == AiType::Military) {
					continue;
				}
				ai_region->ground_strength_full += unit->ground_strength;
				ai_region->air_strength_full += unit->air_strength;
				ai_region->ground_strength += unit->ground_strength;
				ai_region->air_strength += unit->air_strength;
			}
		}

		// Removed a Campaign AI only block, which was causing a crash and an unwanted feature
		// No longer Melee AI only, also times 2 instead of times 1.5
		u32 ground_strength = ai_region->ground_strength_enemy * 2;
		if (ground_strength > ai_region->ground_strength_needed) {
			ai_region->ground_strength_needed = ground_strength;
		}

		u32 air_strength = ai_region->air_strength_enemy * 2;
		if (air_strength > ai_region->air_strength_needed) {
			ai_region->air_strength_needed = air_strength;
		}

		u16 max_force = g_aiscript_controller[ai_region->player].ai_max_force;
		if (max_force) {
			if (ai_region->ground_strength_needed > max_force) {
				ai_region->ground_strength_needed = max_force;
			}
			if (ai_region->air_strength_needed > max_force) {
				ai_region->air_strength_needed = max_force;
			}
		}
	}	

	// Hooked to change functionality later - Pr0nogo
	Unit* get_closest_enemy_structure(s32 x, s32 y, s32 player) { // 004A0F90
		Point16 region_center = scbw::get_region_center(scbw::get_region(helper::get_region_id(x, y)));
		u32 max_distance = 99999999;
		Unit* best_unit = 0;
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit->main_order_id && unit->status & UnitStatus::GroundedBuilding) {
				if (!scbw::is_allied_to(player, unit->player)) {
					u32 distance = scbw::get_distance_fast(region_center, unit->position);
					if (distance < max_distance) {
						max_distance = distance;
						best_unit = unit;
					}
				}
			}
		}
		return best_unit;
	}	

	s16 get_region_id_from_unit(Unit* unit) { // 0049CB40
		if (!unit || !unit->sprite) {
			return false;
		}
		return helper::get_region_id(unit->position.x, unit->position.y);
	}

	bool get_best_worker_for_repair_proc(Unit* repairman, Unit* repair_target) { // 004402B0
		WorkerAi* repairman_ai = (WorkerAi*)repairman->unit_ai;
		if (repairman->player == repair_target->player
			&& units_dat::base_property[repairman->id] & UnitProperty::Worker
			&& !(repairman->status & UnitStatus::InTransport)
			&& !(repairman->status & UnitStatus::InBuilding) // new
			&& repairman_ai && repairman_ai->town
			&& repairman_ai->town->building_scv) { // new
			return repairman_ai->town->building_scv == repairman;
		}
		return false;
	}

	// Hooked to add sprite nullcheck - Pr0nogo
	b32 ai_military_should_wait_for_group(Unit* unit) { // 0043AA10
		// New code
		if (!unit || !unit->sprite) {
			return false;
		}

		MilitaryAi* unit_ai = (MilitaryAi*)unit->unit_ai;
		if (!unit_ai) {
			return false;
		}
		if (unit_ai->type != AiType::Military) {
			return false;
		}
		if (unit_ai->unit != unit) {
			return false;
		}
		if (units_dat::base_property[unit->id] & UnitProperty::Building) {
			return false;
		}
		if (unit_ai->region->state == RegionState::Attacking) {
			return false;
		}

		MilitaryAi* military_ai = unit_ai->region->military.first;
		for (; military_ai; military_ai = military_ai->link.prev) {
			Unit* unit_it = (Unit*)military_ai->unit;
			if (unit_it && unit_it->sprite && unit_it != unit) {
				u32 distance = helper::get_distance_fast(unit_it->position.x, unit->position.x, unit_it->position.y, unit->position.y);
				if  (distance <= 384
				&&   helper::ai_are_on_connected_regions(unit_it, unit)
				&& !(helper::ai_check_military_grouping(unit_it)) && distance > 384)
				{
					return 1;
				}
			}
		}

		// Not sure if this is correct
		if (military_ai) {
			return true;
		}
		return false;
	}

	// Hooked to add more functionality. - Pr0nogo
	// TODO: Add a conditional timeout for deleting towns, somewhere.
	AiTown* delete_ai_town_if_needed(AiTown* town) { // 004347E0
		AiTown* result = helper::try_begin_delete_town(town);
		if (result) {
			helper::delete_aiscripts_using_town(town);
			for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
				if (unit->sprite) {
					if (unit->main_order_id) {
						if (units_dat::base_property[unit->id] & UnitProperty::ResourceContainer) {
							unit->building.resource.resarea = 0;
						}
					}
				}
			}

			u8 resarea = town->resarea;
			if (resarea) {
				g_base_array[resarea].flags &= ~0x2;
			}

			s32 player = (u8)town->player;
			if (g_ai_town_list[player].first == town) {
				g_ai_town_list[player].first = town->prev;
			}

			if (town->next) {
				town->next->prev = town->prev;
			}

			if (town->prev) {
				town->prev->next = town->next;
			}

			town->next = 0;
			town->prev = g_ai_town_list[player].array->first_free;
			if (g_ai_town_list[player].array->first_free) {
				g_ai_town_list[player].array->first_free->next = town;
			}
			g_ai_town_list[player].array->first_free = town;

			reset_town_ext(town);
		}
		return result;
	}

	// Hooked because why not? - Pr0nogo
	bool ai_can_mine_extra(Unit* unit) { // 00447A70
		if (!unit->unit_ai) {
			return 0;
		}
		u8 player = unit->player;
		if (g_aiscript_controller[player].difficulty != 2) {
			return 0;
		}

		u16 unit_id = UnitId::None;
		// TODO: Add support for custom races. -Pr0nogo
		u8 race_id = g_player_table[player].race;
		switch(race_id) {
			case RaceId::Terran:
			{
				unit_id = UnitId::TerranMason;
			} break;
			case RaceId::Protoss:
			{
				unit_id = UnitId::ProtossScribe;
			} break;
			case RaceId::Zerg:
			{
				unit_id = UnitId::ZergDroleth;
			} break;
		}
		u32 completed_count = g_completed_unit_count_table[unit_id].player[player];
		// Originally had an inlined function that combined sieged and unsieged tanks for unit counts
		return completed_count >= 12;
	}

	// Hooked to fix crashes - Pr0nogo
	bool ai_is_building_safe(Unit* unit) { // 00438910
		if (unit) {
			if (unit->main_order_id) {
				s16 regionId = helper::get_region_id(unit->position.x, unit->position.y);
				AiRegion* playerRegions = g_ai_region_captains[unit->player];
				char regionState = playerRegions[regionId].state;
				AiRegion* region = &playerRegions[regionId];
				if (regionState != RegionState::EnemySighted && !region->air_target && !region->ground_target && !(region->flags & 0x20)) {
					return 1;
				}
			}
		}
		return 0;
	}

	// Hooked to fix crashes - Pr0nogo
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2) { // 00438E10
		if (!unit1 || !unit2) {
			return 0;
		}
		u16 region_id = helper::get_region_id(unit2->position.x, unit2->position.y);
		u16 region_id2 = helper::get_region_id(unit1->position.x, unit1->position.y);
		return helper::ai_regions_are_connected(unit2->player, region_id2, region_id);
	}

	// Hooked to change default behaviors - Pr0nogo
	s32 get_bunker_placement_position(u32 placement_id, u32* region_id) { // 004442E0
		u32 region_max = *g_region_ids_count;
		if (!g_region_ids_count) {
			return 0;
		}

		s32 result;
		if (*g_region_ids_count == 1) {
			*region_id = g_region_ids[0];
			result = 1;
		} else {
			u32 iterator1 = 0;
			Point16 region_positions[24];
			u32 tile_distances[24];
			if (g_region_ids_count) {
				memset(tile_distances, 99999999, *g_region_ids_count);
				RegionArray* _paths = *g_sai_paths;
				do {
					u32 region_offset = g_region_ids[iterator1] << 6;
					u32 region_x = *(u32*)((char*)&_paths->regions[0].region_center_x + region_offset) >> 8;
					u32 region_y = *(u32*)((char*)&_paths->regions[0].region_center_y + region_offset) >> 8;
					region_positions[iterator1].x = region_x;
					region_positions[iterator1++].y = region_y;
				} while (iterator1 < region_max);
			}

			Unit* i;
			for (i = *first_player_unit[placement_id].unit; i; i = i->player_link.next) {
				if (i->sprite) {
					if (i->main_order_id) {
						if (i->id == UnitId::TerranAnchor || i->id == UnitId::ZergCagrantColony || i->id == UnitId::ZergSurkithColony || i->id == UnitId::ZergSkortrithColony) {
							u32 iterator2 = 0;
							if (region_max) {
								do { // Distance from unit to region position
									u32 new_tile_distance = helper::get_tile_distance(i->sprite->position.x, i->sprite->position.y, region_positions[iterator2].x, region_positions[iterator2].y);
									if (new_tile_distance < tile_distances[iterator2])
									{
										tile_distances[iterator2] = new_tile_distance;
									}
									region_max = *g_region_ids_count;
									++iterator2;
								} while (iterator2 < *g_region_ids_count);
							}
						}
					}
				}
			}

			u32 best_tile_distance = tile_distances[0];
			u32 current_region_id = 1;
			u32 best_region_id;
			for (best_region_id = 0; current_region_id < region_max; ++current_region_id) {
				if (tile_distances[current_region_id] > best_tile_distance) {
					best_tile_distance = tile_distances[current_region_id];
					best_region_id = current_region_id;
				}
			}
			*region_id = g_region_ids[best_region_id];
			result = 1;
		}
		return result;
	}

	// Hooked to change default behaviors - Pr0nogo
	void ai_get_build_location(u32 unit_id, char* placement_data, u32 player, Point16 desired_pos, s32 max_range) { // 004461B0
		memset(placement_data, 0, 0x1000u);
		helper::ai_get_building_placement_data(max_range, unit_id, placement_data, player, desired_pos);
		helper::ai_remove_unreachable_regions(placement_data, desired_pos);
		if (unit_id != UnitId::ProtossPylon)
		{
			helper::ai_remove_dangerous_regions(unit_id, placement_data, player, desired_pos);
		}
		helper::ai_remove_far_tiles(placement_data, desired_pos, max_range);

		u32 iterator2 = 0;
		while (1) {
			for (u32 i = 0; i < 64; i++) {
				if (placement_data[i]) {
					helper::ai_remove_different_height(placement_data, desired_pos);
					helper::ai_placement_related_something(unit_id, placement_data, desired_pos);
					if (units_dat::base_property[unit_id] & UnitProperty::ResourceDepot) { // Was explicit CC/Nexus/Hatchery check - may have to forbid Hatchery morphs?
						max_range = 1;
					} else {
						max_range = 0;
						helper::ai_remove_unfit_tiles(unit_id, placement_data, player, desired_pos);
					}
					helper::ai_remove_addon_places(unit_id, placement_data, player, desired_pos);
					switch (unit_id) {
						case UnitId::TerranWatchdog:
						case UnitId::ProtossWarden:
						{
							helper::get_turret_placement_position(placement_data, player, desired_pos, unit_id, 0);
							return;
						}
						case UnitId::TerranAnchor:
					//	case UnitId::TerranTencendur:
						{
							if (!helper::get_bunker_placement_position(player, &max_range)) {
								helper::get_turret_placement_position(placement_data, player, desired_pos, unit_id, 0);
								return;
							}
						}
						case UnitId::ZergCagrantColony:
						{
							if (!g_aiscript_controller[player].ai_flags.is_secure_finished || !helper::get_bunker_placement_position(player, &max_range)) {
								helper::get_turret_placement_position(placement_data, player, desired_pos, unit_id, 0);
							}
						} break;
						case UnitId::ProtossPylon:
						{
							helper::get_pylon_placement_position(placement_data, player, desired_pos);
						} break;
						default:
						{
							if (!max_range) {
								helper::get_default_placement_position(unit_id, placement_data, player, desired_pos);
							}
						} break;
					}
					return;
				}
			}
			++iterator2;
			placement_data += 64;
			if (iterator2 < 64) {
				continue;
			}
			break;
		}
	}

	// Hooked to change default behaviors - Pr0nogo
	u32 get_turret_placement_position(char* placementData, u32 player, Point16 desiredPos, u16 unit_id, u32 unkFlag) { // 00443AC0
		// will do this later
		return 0;
	}

	// Hooked to nullcheck sprites - Pr0nogo
	bool ai_check_military_grouping(Unit* unit) { // 00439550
		UnitAi* unitAi = (UnitAi*)unit->unit_ai;
		if (!unitAi || unitAi->type != 4 || units_dat::base_property[unit->id] & 1) {
			return 1;
		}

		MilitaryAi* militaryAi = (MilitaryAi*)unit->unit_ai;
		if (militaryAi->unit != unit) {
			return 1;
		}

		AiRegion* region = militaryAi->region;
		Unit* slowestUnitVar = region->slowest_unit;
		if (!slowestUnitVar
		||	!slowestUnitVar->sprite // new
		||	slowestUnitVar->status & UnitStatus::InTransport
		||	slowestUnitVar == unit
		||	region->state == 2
		||	(u16)(&(*g_sai_paths)->region_count + 32 * (helper::get_region_id_from_unit(region->slowest_unit) + 4392)) < 60u
		||	!helper::ai_are_on_connected_regions(slowestUnitVar, unit)
		||	helper::get_distance_fast(region->slowest_unit->sprite->position.x, unit->sprite->position.x, region->slowest_unit->sprite->position.y, unit->sprite->position.y) <= 0x80) {
			return 1;
		}

		u32 x2 = 0;
		u32 y2 = 0;
		helper::get_region_center_from_id(region->region, &x2, &y2);
		return helper::get_distance_fast(unit->sprite->position.x, x2, unit->sprite->position.y, y2) <= 0x180;
	}

	// Hooked to fix AI height stupidity... or DUMBNESS??? -Pr0nogo
	u32 __stdcall ai_remove_different_height(char* placement_data, Point16 desired_pos) { // 004436C0
		return 0;

		/*
		u32 ground_height;
		char* placement_data_local;
		s32 pos_x_copy;
		u32 placement_data_iterator;
		u32 ground_height_2;
		bool iterate_done;
		u32 loop_var;
		char* placement_data_local_2;
		u32 result;
		char placement_data_buffer[4096];
		s32 pos_x;
		s32 times_to_iterate;
		s32 pos_y;

		memcpy(placement_data_buffer, placement_data, sizeof(placement_data_buffer));
		ground_height = helper::get_ground_height_at_px(desired_pos.y, desired_pos.x);
		pos_x = desired_pos.x - 1024;
		pos_y = desired_pos.y - 1024;
		placement_data_local = placement_data;
		times_to_iterate = 64;
		do
		{
			pos_x_copy = pos_x;
			placement_data_iterator = 0;
			do
			{
				if (placement_data_local[placement_data_iterator])
				{
					ground_height_2 = helper::get_ground_height_at_px(pos_y, pos_x_copy);
					if (ground_height != ground_height_2)
					{
						placement_data_local[placement_data_iterator] = 0;
					}
				}
				++placement_data_iterator;
				pos_x_copy += 32;
			} while (placement_data_iterator < 0x40);
			placement_data_local += 64;
			iterate_done = times_to_iterate == 1;
			pos_y += 32;
			--times_to_iterate;
		} while (!iterate_done);
		loop_var = 0;
		placement_data_local_2 = placement_data;
	LABEL_9:
		result = 0;
		while (!placement_data_local_2[result])
		{
			if (++result >= 0x40)
			{
				++loop_var;
				placement_data_local_2 += 64;
				if (loop_var < 0x40)
				{
					goto LABEL_9;
				}
				memcpy(placement_data, placement_data_buffer, 0x1000u);
				return result;
			}
		}
		return result;
		*/
	}

	// Hooked to investigate removing AI ally retaliation. (No cigar!) -Pr0nogo
	s32 attack_recent_attacker(Unit* unit) { // 004770E0
		Unit* target_unit = unit->auto_target_unit;
		if (!target_unit || unit->unit_ai) {
			return 0;
		}

		u8 target_player = target_unit->player;
		if (target_unit->player == 11) { // Neutral
			target_player = target_unit->sprite->player;
		}
		if (!scbw::is_allied_to(unit->player, target_player)) {
			if (helper::unit_can_attack_target(target_unit, unit, 1)) {
				helper::order_to_attack_target(unit->auto_target_unit, unit, 1, 0);
				unit->status |= UnitStatus::Disabled;
				return 1;
			}
		}
		unit->auto_target_unit = 0;
		return 0;
	}

	u32 ai_get_vespene_type(Unit* unit, u32 player_id, bool for_harvesting) { // 004326E0
		if (!unit || !unit->sprite || !unit->main_order_id) {
			return 0;
		}
		if (!for_harvesting && unit->id == UnitId::ResourceVespeneGeyser) {
			return 1;
		}
		if (unit_id_is_vespene_node(unit->id)) {
			return 2 + (unit->building.resource.resource_amount != 0);
		}
		return 0;
	}

	Unit* get_ai_refinery(AiTown* town, u32 gas_state, u32 index, bool for_harvesting) { // 00432B90
		/*
		Unit* gas_building = town->gas_buildings[0];
		u32 counter = 0;
		if (gas_building && ai_get_vespene_type(town->gas_buildings[0], town->player, for_harvesting) == gas_state && (counter = 1, index == 1)
			|| (gas_building = town->gas_buildings[1]) != 0 && ai_get_vespene_type(town->gas_buildings[1], town->player, for_harvesting) == gas_state && (++counter, counter == index)
			|| (gas_building = town->gas_buildings[2]) != 0 && ai_get_vespene_type(town->gas_buildings[2], town->player, for_harvesting) == gas_state && counter + 1 == index) {
			return gas_building;
		}
		*/

		AiTownExt* town_ext = get_town_ext(town);
		u32 counter = 0;
		for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
			Unit* gas_building = town_ext->gas_buildings[i];
			if (ai_get_vespene_type(gas_building, town->player, for_harvesting) == gas_state && get_gas_tier(gas_building) == 4) {
				if (index == ++counter) {
					return gas_building;
				}
			}
		}
		for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
			Unit* gas_building = town_ext->gas_buildings[i];
			if (ai_get_vespene_type(gas_building, town->player, for_harvesting) == gas_state && get_gas_tier(gas_building) == 3) {
				if (index == ++counter) {
					return gas_building;
				}
			}
		}
		for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
			Unit* gas_building = town_ext->gas_buildings[i];
			if (ai_get_vespene_type(gas_building, town->player, for_harvesting) == gas_state && get_gas_tier(gas_building) == 2) {
				if (index == ++counter) {
					return gas_building;
				}
			}
		}
		for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
			Unit* gas_building = town_ext->gas_buildings[i];
			if (ai_get_vespene_type(gas_building, town->player, for_harvesting) == gas_state && get_gas_tier(gas_building) == 1) {
				if (index == ++counter) {
					return gas_building;
				}
			}
		}

		return 0;
	}

	static Unit* get_best_mineral_field(AiTown* ai_town) {
		u32 best_score = 0;
		u32 best_distance = UINT_MAX;
		Unit* best_mineral_field = 0;
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit->id < UnitId::ResourceMineralField || unit->id > UnitId::ResourceMineralFieldType3) {
				continue;
			}
			if (unit->building.resource.resarea != ai_town->resarea) {
				continue;
			}

			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit_ext->values[ValueAssignedHarvesters] >= 2) {
				continue;
			}

			u32 score = get_harvest_amount(unit);
			if (unit_ext->values[ValueAssignedHarvesters]) {
				score /= 2;
			}

			u32 distance = ai_town->main_building ? scbw::get_distance_fast(ai_town->main_building->position, unit->position) : 0;
			if (score > best_score || (score == best_score && distance < best_distance)) {
				best_score = score;
				best_distance = distance;
				best_mineral_field = unit;
			}
		}
		return best_mineral_field;
	}

	static Unit* get_best_gas_refinery(AiTown* ai_town) {
		u32 best_score = 0;
		u32 best_distance = UINT_MAX;
		Unit* best_gas_refinery = 0;
		AiTownExt* town_ext = get_town_ext(ai_town);
		for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
			Unit* unit = town_ext->gas_buildings[i];
			if (!(unit->status & UnitStatus::Completed)) {
				continue;
			}
			if (unit->building.resource.resarea != ai_town->resarea) {
				continue;
			}

			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit_ext->values[ValueAssignedAiGasHarvesters] >= 2) {
				continue;
			}

			u32 score = get_harvest_amount(unit);
			if (unit_ext->values[ValueAssignedAiGasHarvesters]) {
				score /= 2;
			}

			u32 distance = ai_town->main_building ? scbw::get_distance_fast(ai_town->main_building->position, unit->position) : 0;
			if (score > best_score || (score == best_score && distance < best_distance)) {
				best_score = score;
				best_distance = distance;
				best_gas_refinery = unit;
			}
		}
		return best_gas_refinery;
	}

	// Hooked to help with AI implementation of the vespene overhaul. -Pr0nogo
	void ai_worker_controller(Unit* worker) { // 00435210
		WorkerAi* worker_ai = (WorkerAi*)worker->unit_ai;
		if (worker_ai) {
			if (worker_ai->wait_timer) {
				worker_ai->wait_timer--;
				return;
			}
			Point16 worker_pos = {};
			AiTown* town = worker_ai->town;
			worker_ai->last_update_second = *g_elapsed_time_seconds;
			GroupFlag group_flags = units_dat::group_flags[worker->id];
			if ((group_flags.is_zerg || group_flags.is_protoss || !group_flags.is_terran || !helper::ai_worker_repair_controller(worker, town))
				&& !helper::ai_try_progress_spending_queue_worker(town, worker)) {
				Unit* main_building = town->main_building;
				if (main_building) {
					Unit* town_gas = 0;
					Unit* town_gas_2 = 0;
					if (helper::unit_is_active_resource_depot(main_building)) {
						Unit* best_mineral_field = get_best_mineral_field(town); // New -Veeq7
						if (best_mineral_field) {
							town->mineral = best_mineral_field;
						}

						if (worker_ai->target_resource == AiTargetResource_Mineral) {
							Unit* target_resource = worker->worker.target_resource.unit;
							if (target_resource && helper::unit_is_mineral_field(target_resource)) {
								helper::ai_auto_harvest(target_resource, worker, 0);
							} else {
								Unit* town_mineral = town->mineral;
								if (!town_mineral || town_mineral->building.resource.resource_belongs_to_ai) {
									worker_pos = worker->sprite->position;
									town_mineral = helper::harvest_next_nearest_resources_ex(town->worker_count <= 0xCu, worker, worker_pos);
									if (best_mineral_field) { // New -Veeq7
										town_mineral = best_mineral_field;
									}
									if (!town_mineral) {
										town_mineral = town->mineral;
									}
								}
								helper::ai_auto_harvest(town_mineral, worker, 0);
							}
						} else if (worker_ai->target_resource == AiTargetResource_Gas) {
							Unit* ai_refinery = get_best_gas_refinery(town);
							helper::ai_auto_harvest(ai_refinery, worker, town->mineral);
						}
					}
				}
			}
		}
	}

	static bool unit_has_order(Unit* unit, u32 order_id) {
		if (unit->main_order_id == order_id) {
			return true;
		}
		for (Order* order = unit->order_queue_head; order; order = order->next) {
			if (order->order_id == order_id) {
				return true;
			}
		}
		return false;
	}

	static bool unit_has_uninterruptable_order(Unit* unit) {
		if (!orders_dat::can_be_interrupted[unit->main_order_id]) {
			return true;
		}
		for (Order* order = unit->order_queue_head; order; order = order->next) {
			if (!orders_dat::can_be_interrupted[order->order_id]) {
				return true;
			}
		}
		return false;
	}

	// Hooked to improve AI gas harvesting. -Pr0nogo
	Unit* get_resource_area_gas(u32 resarea_id, AiTown* town) { // based on 00444460
		AiTownExt* town_ext = get_town_ext(town);
		town_ext->gas_buildings_count = 0;

		ResourceArea* resarea = &g_base_array[resarea_id];
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit->sprite && unit->main_order_id && units_dat::base_property[unit->id] & UnitProperty::ResourceContainer) {
				if (!unit_id_is_minerals(unit->id) && unit->building.resource.resarea == resarea_id) {
					if (town_ext->gas_buildings_count < GasBuildingsCap) {
						town_ext->gas_buildings[town_ext->gas_buildings_count++] = unit;
					}
				}
			}
		}
		sort_gas_buildings(town);
		return 0;
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	char ai_reassign_worker_resources(AiTown* town) { // 00435900, unused int arg
		u8 result = 0;
		if (town->inited) {
			if (!town->resarea) {
				AiLayoutsTown* layouts_town = get_layouts_town(town);
				if (!layouts_town->inited || !layouts_town->wants_no_resarea) {
					town->resarea = helper::get_closest_base_from_startloc(town->player, town->position.x, town->position.y);
					town->resource_units_not_set = 1;
				}
			}

			AiTownExt* town_ext = get_town_ext(town);
			u32 resarea = town->resarea;
			if (town->resource_units_not_set) {
				town->resource_units_not_set = 0;
				get_resource_area_gas(resarea, town);
				sort_gas_buildings(town);
				for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
					helper::ai_make_guard(town->player, town_ext->gas_buildings[i]);
				}
			}
			if (!town->mineral) {
				Unit* resarea_minerals = helper::get_resource_area_mineral(town->resarea);
				town->mineral = resarea_minerals;
				if (resarea_minerals) {
					helper::ai_make_guard(town->player, resarea_minerals);
				}
			}

			if (resarea) {
				ResourceArea* ai_resarea = &g_base_array[resarea];
				u32 new_worker_limit = 0;
				if (resarea && resarea < 250) {
					new_worker_limit = 2.25 * ai_resarea->mineral_field_count + 2 * ai_resarea->geyser_count;
				}
				town->worker_limit = new_worker_limit;
			}
			u8 mineral_value = 1;
			u8 gas_value = 2;
			if (get_ai_refinery(town, 3, 1, true) || get_ai_refinery(town, 2, 1, true)) {
				if (!town->mineral) {
					mineral_value = 2;
				}
			} else {
				gas_value = 1;
			}

			u32 town_player = town->player;
			u8 minerals_percentage = 0;
			u8 gas_percentage = 0;
			helper::ai_resource_need_split(&gas_percentage, town_player, &minerals_percentage);
			u32 gas_count = gas_percentage * town->worker_count / 100.0;

			u32 worker_count = 0;
			// TODO: Add support for custom races. -Pr0nogo
			switch (g_player_table[town_player].race) {
				case RaceId::Terran: {
					worker_count += g_completed_unit_count_table[UnitId::TerranMason].player[town_player];
					worker_count += g_completed_unit_count_table[UnitId::TerranTalos].player[town_player];
				} break;
				case RaceId::Protoss: {
					worker_count += g_completed_unit_count_table[UnitId::ProtossScribe].player[town_player];
					worker_count += g_completed_unit_count_table[UnitId::ProtossArtisan].player[town_player];
				} break;
				case RaceId::Zerg: {
					worker_count += g_completed_unit_count_table[UnitId::ZergDroleth].player[town_player];
					worker_count += g_completed_unit_count_table[UnitId::ZergGosvileth].player[town_player];
				} break;
			}

			AI_Main* ai_player = &g_aiscript_controller[town_player];
			s32 gas_delta = ai_player->vespene_needed - ai_player->vespene_available;
			u32 new_gas_count = gas_delta / 150 + worker_count / 7;
			if (gas_count < new_gas_count) {
				gas_count = new_gas_count;
			}
			if (gas_count > worker_count / 3) {
				gas_count = worker_count / 3;
			}

			// New limit
			u32 max_gas_worker_count = 0;
			for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
				Unit* gas = town_ext->gas_buildings[i];
				if (gas->status & UnitStatus::Completed) {
					max_gas_worker_count += 2;
				}
			}
			if (gas_count > max_gas_worker_count) {
				gas_count = max_gas_worker_count;
			}

			for (WorkerAi* worker_ai = town->workers.first; worker_ai; worker_ai = worker_ai->link.prev) {
				Unit* worker = worker_ai->parent;
				u8 unit_order = worker->main_order_id;
				if (unit_order == OrderId::ConstructingBuilding
				|| unit_order == OrderId::Pickup4
				|| unit_order == OrderId::DroneBuild
				|| worker->status & UnitStatus::InTransport) {
					worker_ai->last_update_second = *g_elapsed_time_seconds;
				//} else if (*g_elapsed_time_seconds - worker_ai->last_update_second >= 60) {
				//	helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
				//	worker_ai->wait_timer = 10;
				//	worker_ai->last_update_second = *g_elapsed_time_seconds;
				//	worker_ai->reassign_count += 1;
				//	helper::ai_get_oldpeons(worker_ai->town, 1u);
				} else {
					u32 timer = 5;
					if (unit_has_uninterruptable_order(worker)
						|| worker->main_order_id == OrderId::CanHarvestOre
						|| worker->main_order_id == OrderId::MiningMinerals
						|| worker->main_order_id == OrderId::ReturnMinerals
						|| worker->main_order_id == OrderId::CanEnterGasMine
						|| worker->main_order_id == OrderId::HarvestGas3
						|| worker->main_order_id == OrderId::ReturnGas
						|| worker->main_order_id == OrderId::HarvestInterrupt
						|| worker->main_order_id == OrderId::HarvestFinish
						|| unit_has_order(worker, OrderId::BuildTerran)
						|| unit_has_order(worker, OrderId::BuildProtoss1)
						|| unit_has_order(worker, OrderId::BuildProtoss2)
						|| unit_has_order(worker, OrderId::DroneStartBuild)
						|| unit_has_order(worker, OrderId::DroneBuild)) {
						timer = 30;
					}

					UnitExt* workerExt = get_unit_ext(worker);
					if (*g_elapsed_time_seconds - worker_ai->last_update_second >= timer
						&& !workerExt->timers[TimerAI_KitingLeash])
					{
						helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
						worker_ai->wait_timer = 0;
						worker_ai->last_update_second = *g_elapsed_time_seconds;
						helper::ai_get_old_peons(worker_ai->town, 1);
					}
				}

				if (worker->status & UnitStatus::Completed) {
					u8* target_resource_type = gas_count ? &gas_value : &mineral_value;
					if (gas_count) {
						--gas_count;
					}
					if (*target_resource_type == 2) {
						if (max_gas_worker_count) {
							max_gas_worker_count--;
						} else {
							*target_resource_type = 1;
						}
					}
					worker_ai->target_resource = *target_resource_type;
					result = *target_resource_type;
				}
			}
		}
		return result;
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback) { // 00433B90
		if (worker->resource_type & ResourceType::Minerals) {
			worker->user_action_flags |= 1u;
			if (worker->main_order_id) {
				while (1) {
					if (!worker->order_queue_tail) {
						break;
					}
					if (!orders_dat::can_be_interrupted[worker->order_queue_tail->order_id]
					&&	worker->order_queue_tail->order_id != OrderId::ReturnMinerals) {
						break;
					}
					helper::remove_order_from_unit_queue(worker->order_queue_tail, worker);
				}
				helper::perform_another_order(OrderId::ReturnMinerals, 0, worker, 228, {}, 0);
			}
			helper::do_next_queued_order(worker);
		} else if (worker->resource_type & ResourceType::VespeneGas) {
			helper::issue_order_targeting_nothing(worker, OrderId::ReturnGas);
		} else {
			if (!resource || !(resource->status & UnitStatus::Completed)) {
				resource = fallback;
			}
			if (resource && (resource->id == UnitId::ResourceVespeneRidge || resource->id == UnitId::ResourceVespeneGeyser || resource->status & UnitStatus::Completed)) {
				helper::assign_order_with_target(resource, worker, OrderId::Harvest1);
				resource->building.resource.resource_belongs_to_ai = 1;

				UnitExt* resource_ext = get_unit_ext(resource);
				resource_ext->values[ValueAssignedHarvesters] += 1; // Update assigned harvesters for this frame for the sake of mineral field priority calcs...
			}
		}
	}

	// Hooked to replace old functionality with new, to prioritize Taloses over Masons for AI repairs and to allow more than 1 repairer. -Pr0nogo
	Unit* ai_find_best_repair_unit(Unit* to_repair) { // 00443160
		EntryArray workers = {};
		unit_search_radial(&workers, to_repair->position, 64 * 32);
		unit_search_filter(&workers, to_repair, FilterAlly, {UnitId::TerranMason, UnitId::TerranTalos});
		Unit* best = unit_search_find_best(&workers, [=](Unit* worker) {
			if (!worker->has_path_to_unit(to_repair) ||
				to_repair->order_target.unit == worker ||
				scbw::is_allied_to(worker->player, to_repair->player) ||
				!helper::is_order_valid(OrderId::Repair1, worker, 0, to_repair, 0)) {
				return ScoreInvalidUnit;
			}
			s32 score = 0;
			score -= scbw::get_distance_fast(worker->position, to_repair->position);
			if (worker->id == UnitId::TerranTalos) {
				score += 1000;
			}
		});
		return best;
	}

	// Same as above. -Pr0nogo
	s32 ai_worker_repair_controller(Unit* worker, AiTown* town) { // 00433C70
		u32 best_score = 0;
		Unit* repairee = 0;
		for (BuildingAi* building_ai = town->buildings.first; building_ai; building_ai = building_ai->link.prev) {
			Unit* building = building_ai->parent;
			if (building->connected_unit || !helper::ai_is_building_safe(building)) {
				continue;
			}
			if (!(building->status & UnitStatus::Completed)) {
				continue; // Used to break, but it should actually continue. unless the building list is sorted a particular way, which allows to break early...?
			}
			s32 max_hp = units_dat::max_hitpoints[building->id] / 256;
			if (!max_hp) {
				max_hp = (building->hitpoints + 255) / 256;
				if (!max_hp) {
					max_hp = 1;
				}
			}
			u32 score = max_hp - ((building->hitpoints + 255) / 256);
			if (best_score < score) {
				repairee = building;
				best_score = score;
			}
		}
		if (!town->buildings.first || !repairee) {
			if (!town->building_scv) {
				town->building_scv = worker;
			}
			if (town->building_scv != worker) {
				return 0;
			}
			repairee = helper::ai_find_best_repair_unit(worker); // This writing to repairee seems really wrong
			if (!repairee) {
				return 0;
			}
		}
		worker->user_action_flags |= 1u;
		Point16 repairee_pos = {}; // Shouldn't we return when nothing to repair?
		if (repairee) {
			repairee_pos = repairee->sprite->position;
		}
		if (worker->main_order_id) {
			for (Order* order_tail = worker->order_queue_tail; order_tail; order_tail = worker->order_queue_tail) {
				if (!orders_dat::can_be_interrupted[order_tail->order_id] && order_tail->order_id != OrderId::Repair1) {
					break;
				}
				helper::remove_order_from_unit_queue(order_tail, worker);
			}
			helper::perform_another_order(OrderId::Repair1, 0, worker, UnitId::None, repairee_pos, repairee);
		}
		helper::do_next_queued_order(worker);
		repairee->connected_unit = worker;
		return 1;
	}

	bool ai_resarea_valid_for_expo(u32 player, u32 resarea, AiTown* main_town, bool has_transport, EntryArray* out_units) {
		ResourceArea* area = &g_resource_areas->areas[resarea];
		if (area->flags & 2) { // Already taken by an AI, maybe we shouldn't care and allow them both to compete for the expo?
			return false;
		}
		Region* area_region = scbw::get_region(scbw::get_region_id(area->position));
		Region* main_region = scbw::get_region(scbw::get_region_id(main_town->position));
		if (main_region->group_index != area_region->group_index) {
			if (!has_transport) {
				return false;
			}
		}

		// Since we are not checking for area->flags & 1, we need to make sure the AI doesn't expand to own main town and main towns of allied players
		// Maybe we should assign the area flag 2 for those? Or maybe not check area flag 2 at all and only use this method?
		for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
			if (scbw::is_allied_to(player, other_player) && g_player_table[other_player].type == PlayerType::Computer) {
				for (AiTown* town = g_ai_town_list[other_player].first; town; town = town->prev) {
					if (scbw::get_distance_fast(town->position, area->position) <= 12 * 32) {
						return false;
					}
				}
			}
		}

		// A bit annoying to have to this like that, but it's better than doing the same search twice (ai_resarea_valid_for_expo and get_resarea_score)
		EntryArray local_units = {};
		if (!out_units) {
			out_units = &local_units;
		}
		unit_search_radial(out_units, area->position, 12 * 32);
		for (u32 j = 0; j < out_units->count; ++j) {
			Unit* unit = out_units->units[j];
			if (unit->player < PlayerLimit) {
				if (player != unit->player && scbw::is_allied_to(player, unit->player)
					&& (units_dat::group_flags[unit->id].is_factory
						|| units_dat::base_property[unit->id] & UnitProperty::ResourceDepot
						|| units_dat::base_property[unit->id] & UnitProperty::Worker)) {
					return false;
				}
				if (!scbw::is_allied_to(player, unit->player) && units_dat::group_flags[unit->id].is_building) {
					return false;
				}
			}
		}
		return true;
	}

	static f32 get_resarea_score(u32 player, AiTown* main_town, ResourceArea* area, EntryArray* units, bool use_absolute_path) {
		f32 score = 0;

		// TODO: make trigger customizable (and probably convert all biases to 1.0, so it's always the default)
		const f32 own_bias      = -1.0;
		const f32 ally_bias     = -0.5;
		const f32 enemy_bias    =  0.5;
		const f32 distance_bias = -5.0;
		const f32 mineral_bias  =  0.5;
		const f32 gas_bias      =  1.5;

		u32 own_count = 0;
		u32 ally_count = 0;
		u32 enemy_count = 0;
		u32 start_location_count = 0;
		f32 average_own_distance = 0; 
		f32 average_ally_distance = 0; 
		f32 average_enemy_distance = 0; 
		f32 average_start_location_distance = 0; 

		for (u32 i = 0; i < PlayerLimit; ++i) {
			for (Unit* unit = first_player_unit->unit[i]; unit; unit = unit->player_link.next) {
				f32 distance = scbw::get_distance_fast(area->position, unit->position) / 32.0;
				if (player == i) {
					own_count++;
					average_own_distance += distance;
				} else if (scbw::is_allied_to(player, i)) {
					ally_count++;
					average_ally_distance += distance;
				} else {
					enemy_count++;
					average_enemy_distance += distance;
				}
			}
			if (player != i && first_player_unit->unit[i]) {
				f32 distance = scbw::get_distance_fast(area->position, g_start_positions[i]) / 32.0;
				start_location_count++;
				average_start_location_distance += distance;
			}
		}
		if (own_count) {
			average_own_distance /= own_count;
			score += average_own_distance * own_bias;
		}
		if (ally_count) {
			average_ally_distance /= ally_count;
			score += average_ally_distance  * ally_bias;
		}
		if (enemy_count) {
			average_enemy_distance /= enemy_count;
			score += average_enemy_distance * enemy_bias;
		}

		f32 distance_from_main = scbw::get_ground_path_distance_tiles(main_town->position, area->position);
		score += distance_from_main * distance_bias;

		f32 total_mineral_yield = 0;
		f32 total_gas_yield = 0;
		for (Unit* unit = *g_first_visible_unit; unit; unit = unit->link.next) {
			if (unit_id_is_minerals(unit->id)) {
				total_mineral_yield += get_harvest_amount(unit);
			}
			if (unit_id_is_gas_structure(unit->id)) {
				total_gas_yield += get_harvest_amount(unit); // should ignore capped (which can happen on an occupied base)
			}
		}
		if (area->mineral_field_count) {
			f32 average_yield = total_mineral_yield / area->mineral_field_count;
			score += average_yield * total_mineral_yield * mineral_bias;
		}
		if (area->geyser_count) {
			f32 average_yield = total_gas_yield / area->geyser_count;
			score += average_yield * total_gas_yield * gas_bias;
		}

		for (u32 j = 0; j < units->count; ++j) {
			Unit* unit = units->units[j];
			if (unit->player < PlayerLimit) {
				if (player != unit->player && scbw::is_allied_to(player, unit->player) && units_dat::group_flags[unit->id].is_building) {
					score -= 10;
				}
			}
		}

		return score;
	}

	static u32 ai_find_expansion_custom(u32 player, AiTown* main_town) {
		u32 best_resarea = 0;
		f32 best_score = INT_MIN;

		bool has_transport = scbw::player_has_transport(player);
		Region* main_region = scbw::get_region(scbw::get_region_id(main_town->position));
		for (u32 i = 1; i <= g_resource_areas->count; ++i) {
			ResourceArea* area = &g_resource_areas->areas[i];
			EntryArray units = {};
			if (!ai_resarea_valid_for_expo(player, i, main_town, has_transport, &units)) {
				continue;
			}
			Region* area_region = scbw::get_region(scbw::get_region_id(area->position));
			bool use_absolute_path = false;
			if (main_region->group_index != area_region->group_index && has_transport) {
				use_absolute_path = true;
			}

			f32 score = get_resarea_score(player, main_town, area, &units, use_absolute_path);
			if (score > best_score) {
				best_score = score;
				best_resarea = i;
			}
		}
		
		if (best_resarea) {
			g_resource_areas->areas[best_resarea].flags |= 2;
		}
		return best_resarea;
	}

	// Hooked to remove a ridicilus condition that would stop AI from expanding in late game and to allow base layouts to control expand order
	void ai_expand(AiScript* aiscript, u32 block, u32 amount) { // 0045B530
		default_ai_expansion_block[aiscript->player] = block;
		if (aiscript->town->resarea) { 
			u32 base_id = ai_get_next_custom_expo(aiscript->town);
			if (!base_id || !helper::ai_try_expanding_to(base_id)) {
				base_id = ai_find_expansion_custom(aiscript->player, aiscript->town);
			}
			if (base_id) {
				helper::ai_expand_to(aiscript, base_id, block);
			}
		}
	}

	// Hooked to add addon cases for new addon buildings and to reserve space around buildings. -Veeq7
	void __stdcall ai_remove_addon_placements(u16 unit_id, PlacementData* placement_data, u32 player, Point16 desired_position) { // 00443CC0
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (unit->sprite && unit->main_order_id != OrderId::Die && units_dat::base_property[unit->id] & UnitProperty::Building) {
				// Map coordinates to the placement grid
				s32 unit_x = unit->sprite->position.x / 32 - desired_position.x / 32 + 32;
				s32 unit_y = unit->sprite->position.y / 32 - desired_position.y / 32 + 32;

				// Dimensions
				u32 w = units_dat::building_dimensions[unit->id].x / 32;
				u32 h = units_dat::building_dimensions[unit->id].y / 32;

				// Go to top left
				unit_x -= (w + 1) / 2;
				unit_y -= (h + 1) / 2;

				// Add padding on top and left
				// Stackable buildings don't have padding between each other
				if (!unit_id_is_stackable_building(unit_id) || !unit_id_is_stackable_building(unit->id)) {
					unit_x -= 1;
					unit_y -= 1;
					w += 1;
					h += 1;
				}

				// Handle addons
				if (unit_id_has_addon(unit->id)) {
					w += 2;
				}
				if (unit_id_has_addon(unit_id)) {
					unit_x -= 2;
					w += 2;
				}

				// Forbid placements
				for (u32 y = unit_y; y < unit_y + h; ++y) {
					for (u32 x = unit_x; x < unit_x + w; ++x) {
						if (x < 0 || x >= 64 || y < 0 || y >= 64) {
							continue;
						}
						placement_data->tile_valid[y][x] = 0;
					}
				}
			}
		}
	}

	static bool is_valid_grouping_unit(Unit* unit) {
		UnitExt* unit_ext = get_unit_ext(unit);
		return unit_ext->damage_dealt_timer >= 4 * 24 
			&& unit_ext->damage_taken_timer >= 4 * 24;
	}

	static bool group_before_region(Unit* unit, AiRegion* ai_region) {
		// Original rule
		if (ai_region->state == RegionState::ContinueAttackHere && helper::ai_has_incomplete_military_in_region(ai_region)) {
			return true;
		}

		return false; // Currently disable the new stuff, since we seem to have issues with grouping? -Veeq7

		// Forbid in-combat units from grouping
		if (!is_valid_grouping_unit(unit)) {
			return false;
		}

		// Don't group before prepare region
		switch (ai_region->state) {
			case RegionState::PreparingAttack:
			case RegionState::PreparingAttackOldUnreachable:
				return false;
		}

		// Group before entering enemy teritory
		switch (ai_region->state) {
			case RegionState::ContinueAttackHere:
			case RegionState::Attacking:
				return true;
		}

		// Group before entering threatning regions
		if (ai_region->ground_strength_enemy || ai_region->air_strength_enemy) {
			return true;
		}

		return false;
	}

	static bool is_valid_group_region(AiRegion* target_region, AiRegion* group_region) {
		if ((!target_region->ground_strength_needed || group_region->ground_strength >= target_region->ground_strength_needed - target_region->ground_strength)
		&&	(!target_region->air_strength_needed    || group_region->air_strength    >= target_region->air_strength_needed    - target_region->air_strength)) {
			return false;
		}
		return true;
	}

	// Hooked to fix AI pathfinding lag and improve army spread. -Pr0nogo
	void ai_manage_military_controller(Unit* unit) { // 0043D910
		if (!unit->unit_ai || unit->unit_ai->type != AiType::Military) {
			return;
		}

		MilitaryAi* military_ai = (MilitaryAi*)unit->unit_ai;
		if (military_ai->unit != unit) {
			return;
		}

		AiRegion* ai_region = military_ai->region;
		if (units_dat::base_property[unit->id] & UnitProperty::Building) {
			return;
		}

		switch (unit->id) {
			case UnitId::TerranMaverick:
			case UnitId::TerranHarakan: {
				if (helper::ai_defender_bunker_behavior(unit, ai_region->region)) {
					return;
				}
			} break;
		}

		if (helper::ai_cast_spell_behavior(unit, 0)) {
			return;
		}

		if (helper::ai_update_attack_target(unit, 0, 1, 0)) {
			return;
		}

		u32 region_id = ai_region->region;
		if (group_before_region(unit, ai_region)) {
			if (!ai_region->slowest_unit) {
				helper::ai_assign_region_to_slowest_unit(ai_region);
			}
			Unit* slowest_unit = ai_region->slowest_unit;
			if (slowest_unit) {
				u32 new_region_id = helper::get_region_id_from_unit(slowest_unit);
				if (is_valid_group_region(ai_region, scbw::get_ai_region(unit->player, new_region_id))) {
					region_id = new_region_id;

				//	scbw::printf("Player %u is grouping units!", unit->player);
				//	Point16 ping = scbw::get_region_center(scbw::get_region(new_region_id));
				//	scbw::minimap_ping(ping.x, ping.y, unit->player);
				}
			}
		}

		Region* region = scbw::get_region(region_id);
		if (helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) == region_id) {
			helper::make_to_hold_position(unit);
		} else {
			Point16 center = scbw::get_region_center(region);
			helper::ai_order_to_destination(unit, OrderId::AttackMove, center.x, center.y);
		}
	}

	// Hooked to help AI group mixed speed armies in more intelligent ways. -Pr0nogo
	void ai_assign_region_to_slowest_unit(AiRegion* ai_region) { // 00436F70
		Region* region = scbw::get_region(ai_region->region);
		Point16 center = scbw::get_region_center(region);

		Unit* slowest_ground_unit = 0;
		u32 slowest_ground_speed = UINT_MAX;
		Unit* slowest_air_unit = 0;
		u32 slowest_air_speed = UINT_MAX;

		Unit* slowest_unit = ai_region->slowest_unit;
		if (slowest_unit) {
			if (is_valid_grouping_unit(slowest_unit)) {
				u32 speed = scbw::get_distance_fast(slowest_unit->position, center); // Used to divide by flingy_top_speed, but we just want to group around the closest unit
				if (slowest_unit->status & UnitStatus::InAir) {
					slowest_air_unit = ai_region->slowest_unit;
					slowest_air_speed = speed;
				} else {
					slowest_ground_unit = ai_region->slowest_unit;
					slowest_ground_speed = speed;
				}
			}
		}

		for (MilitaryAi* military_ai = ai_region->military.first; military_ai; military_ai = military_ai->link.prev) {
			Unit* unit = military_ai->unit;
			if (is_valid_grouping_unit(unit)) {
				u32 speed = scbw::get_distance_fast(unit->position, center); // Used to divide by flingy_top_speed, but we just want to group around the closest unit
				if (unit->status & UnitStatus::InAir) {
					if (speed < slowest_air_speed) {
						slowest_air_speed = speed;
						slowest_air_unit = military_ai->unit;
					}
				} else if (speed < slowest_ground_speed) {
					slowest_ground_speed = speed;
					slowest_ground_unit = military_ai->unit;
				}
			}
		}

		if (slowest_ground_unit) {
			ai_region->slowest_unit = slowest_ground_unit;
		} else {
			ai_region->slowest_unit = slowest_air_unit;
		}
	}

	// Hooked to improve behavior. -Pr0nogo
	void ai_respond_to_cloaked_threat(Unit* victim, Unit* threat) { // 0043C580
		if (victim->unit_ai) {
			g_aiscript_controller[victim->player].ai_flags.is_secure_finished;
			UnitAi* victim_ai = victim->unit_ai;
			AiRegion* victim_region = 0;
			if (victim_ai->type == AiType::Military) {
				MilitaryAi* military_ai = (MilitaryAi*)victim_ai;
				victim_region = military_ai->region;
			} else {
				if (victim_ai->type != AiType::Guard) {
				start:
					Unit* unit = *first_player_unit[victim->player].unit;
					Unit* unit_to_order_1 = 0;
					u32 max_distance = 9999999;
					Unit* unit_2 = *first_player_unit[victim->player].unit;
					if (unit) {
						do {
							if (unit->sprite) {
								u8 first_order_id = unit->main_order_id;
								if (first_order_id) {
									switch (unit->id) {
										case UnitId::TerranSeraph:
										case UnitId::ProtossWitness:
										case UnitId::ZergOvileth:
										case UnitId::ZergIroleth:
										{
											if (helper::is_unit_loaded(unit)) {
												break;
											}
											if (!unit->is_blind
											&&  (first_order_id == OrderId::ComputerAI
												|| first_order_id == OrderId::ComputerReturn
												|| first_order_id == OrderId::GuardPost
												|| first_order_id == OrderId::AIPatrol)) {
												u32 distance = helper::get_distance_fast(victim->sprite->position.x, unit->sprite->position.x, victim->sprite->position.y, unit->sprite->position.y);
												if (distance < max_distance) {
													unit_to_order_1 = unit;
													max_distance = distance;
												}
											}
										} break;
										case UnitId::TerranComsatStation:
										{
											if (!(unit->status & UnitStatus::Completed)) {
												break;
											}
											// TODO: Add check to see if victim is already detected / if there's a detector in the box?
											if (victim->id != UnitId::TerranAnchor || !helper::is_unit_loaded(victim)) {
												if (!helper::ai_unit_can_attack(victim) || units_dat::base_property[victim->id] & UnitProperty::Worker) {
													break;
												}
											}
											if (unit->energy >= techdata_dat::energy_cost[TechId::ScannerSweep] * 256 || unit->main_order_id == OrderId::PlaceScanner) {
												break;
											}
											helper::assign_order_with_target(threat, unit, OrderId::PlaceScanner);
											return;
										} break;
									}
								}
							}
							unit = unit->player_link.next;
							unit_2 = unit;
						} while (unit);
						if (unit_to_order_1) {
							Point16 victim_pos = {};
							victim_pos.x = victim->sprite->position.x;
							victim_pos.y = victim->sprite->position.y;
							unit_to_order_1->user_action_flags |= 1;
							if (unit_to_order_1->main_order_id) {
								while (1) {
									Order* tail_order_1 = unit_to_order_1->order_queue_tail;
									if (!tail_order_1) {
										break;
									}
									if (!orders_dat::can_be_interrupted[tail_order_1->order_id] && tail_order_1->order_id != OrderId::ComputerReturn) {
										break;
									}
									helper::remove_order_from_unit_queue(tail_order_1, unit_to_order_1);
								}
								helper::perform_another_order(OrderId::ComputerReturn, 0, unit_to_order_1, UnitId::None, victim_pos, 0);
							}
							helper::do_next_queued_order(unit_to_order_1);
						}
					}
					return;
				}
				BuildingAi* building_ai = (BuildingAi*)victim_ai;
				victim_region = &g_ai_region_captains[victim->player][helper::get_region_id(building_ai->parent->position.x, building_ai->parent->position.y)]; // WRONG... or is it?
			}
			if (victim_region) {
				if (victim_region->follow_target) {
					helper::assign_order_with_target(victim, victim_region->follow_target, OrderId::AIPatrol);
					return;
				}
			}
			goto start;
		}
	}

	// Hooked to remove computer return bullshit and improve kiting. -Veeq7
	u32 ai_try_return_home(Unit* unit, u32 dont_issue_order) {
		return 0;
	}

	b32 ai_is_unit_id_unit_race(u16 unit_id, Unit* unit) {
		if (unit_id_race_check(unit_id, unit->get_race())) {
			return 1;
		}
		*g_last_internal_error = 25;
		return 0;
	}

	// Hooked to let AI use new transports. -Pr0nogo
	bool player_has_transport(u32 player_id) { // 00437250
		return g_completed_unit_count_table[UnitId::TerranTrojan].player[player_id]
			|| g_completed_unit_count_table[UnitId::TerranPythean].player[player_id]
			|| g_completed_unit_count_table[UnitId::ProtossEnvoy].player[player_id]
			|| g_completed_unit_count_table[UnitId::ZergOvileth].player[player_id]
			|| g_completed_unit_count_table[UnitId::ZergIroleth].player[player_id]
			|| g_completed_unit_count_table[UnitId::ZergOthstoleth].player[player_id];
	}

	u32 request_military_to(u16* defense_ptr, AiRegion* ai_region, u32 against_air)	{ // 0043E760
		/*
		u32 sum = 0;
		for (u16* ptr = defense_ptr; *ptr; ++ptr) {
			u32 unit_id = *ptr - 1;
			if (helper::ai_add_to_region_military(ai_region, unit_id, ai_region->defense_priority)) {
				sum += against_air ? ai_strength_air[unit_id] : ai_strength_ground[unit_id];
			}
		}
		return sum;
		*/

		u32 player = ai_region->player;
		Region* region = scbw::get_region(ai_region->region);
		Point16 region_center = scbw::get_region_center(region);
		s32 missing_ground = ai_region->ground_strength_needed - ai_region->ground_strength_full;
		s32 missing_air = ai_region->air_strength_needed - ai_region->air_strength_full;

		Unit* best_unit = 0;
		f32 best_score = 0;
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (!(unit->status & UnitStatus::Completed) || !helper::is_valid_for(unit->id, unit, 0)) {
				continue;
			}
			
			if (!unit->unit_ai || (unit->unit_ai->type != AiType::Guard && unit->unit_ai->type != AiType::Military)) {
				continue;
			}

			if (unit->unit_ai->type == AiType::Guard && ai_region->defense_priority <= 110) {
				continue;
			}

			u32 force_divisor = 1;
			if (unit->unit_ai->type == AiType::Military) {
				AiRegion* current_ai_region = unit->unit_ai->military_ai.region;
				if (current_ai_region->region == ai_region->region) {
					continue;
				}

				// Skip attack wave units for unimportant defense regions and reduce their score otherwise
				if (scbw::is_attack_region(current_ai_region)) {
					if (ai_region->defense_priority <= 110) {
						continue;
					} else {
						force_divisor = 4;

						// Only request up to half of the needed force from attack waves
						if ((unit->ground_strength && ai_region->ground_strength_full >= (s32)ai_region->ground_strength_needed / 2) ||
							(unit->air_strength    && ai_region->air_strength_full    >= (s32)ai_region->air_strength_needed    / 2)) {
							continue;
						}
					}
				}
				
				// Skip units from more important regions, unless they have excess force
				if (ai_region->defense_priority <= current_ai_region->defense_priority) {
					if ((unit->ground_strength && current_ai_region->ground_strength_full - (s32)unit->ground_strength < (s32)current_ai_region->ground_strength_needed) ||
						(unit->air_strength    && current_ai_region->air_strength_full    - (s32)unit->air_strength    < (s32)current_ai_region->air_strength_needed)) {
						continue;
					}
				}
			}

			// Skip transports
			if (unit_id_is_transport(unit->id)) {
				continue;
			}

			// Skip units that can't access the region
			if (region->accessibility_flags == RegionAirOnly && unit->pathing_flags & 1) {
				continue;
			}
			
			s32 ground_force = std::max(std::min((s32)unit->ground_strength, missing_ground), 0);
			s32 air_force = std::max(std::min((s32)unit->air_strength, missing_air), 0);
			s32 force = ground_force + air_force;
			force /= force_divisor;
			if (!force) {
				continue;
			}

			f32 distance = helper::get_path_distance_to(unit, region_center);
			f32 score = distance >= 1 ? force / distance : force;
			if (score > best_score) {
				best_score = score;
				best_unit = unit;
			}
		}

		if (best_unit && scbw::move_unit_ai(best_unit, ai_region)) {
			return against_air ? ai_strength_air[best_unit->id] : ai_strength_ground[best_unit->id];
		}
		return 0;
	}

	// Hooked to improve functionality and allow Pylons to respect the asc3 "creep" header. -Pr0nogo
	// GREAT GOOGLY MOOGLY BATMAN, THIS IS A DISASTER
	s32 ai_manage_building_placement(Unit* builder, u32 unit_id, Point16 try_position, Point16* out_position, s32 area_tiles) { // 004465C0
		Point16 point1 = {};
		Point16 point2 = {};
		point1.x = try_position.x / 32;
		point2.x = try_position.x / 32;
		u32 player_var = builder->player;
		point1.y = (*(s32*)&try_position >> 16) / 32;
		point2.y = point1.y;
		u32 player = player_var;
		Point16 fallback = {};
		u32 fallback_y = 0;

		u32 addon_cucko0 = 0;
		s32* addon_cucko1 = 0;
		u16 addon_cucko2 = 0;
		u16* addon_cucko3 = 0;
		u16 addon_cucko4 = 0;
		u32 addon_dims_x1 = 0;
		u32 addon_dims_x2 = 0;
		if (units_dat::base_property[unit_id] & UnitProperty::Addon) {
			addon_cucko0 = 4 * (u16)builder->id;
			addon_dims_x1 = *(u16*)((char*)&units_dat::building_dimensions[0].x + addon_cucko0);
			fallback = point2;
			addon_cucko0 = *(u16*)((char*)&units_dat::building_dimensions[0].y + addon_cucko0) / -2 / 32 + point2.y;
			addon_cucko1 = &units_dat::max_hitpoints[unit_id + 122];
			addon_cucko2 = *(u16*)addon_cucko1 / 32 + point2.x - addon_dims_x1 / 2 / 32;
			addon_cucko3 = (u16*)((char*)&units_dat::max_hitpoints[unit_id + 122] + 2);
			fallback_y = *addon_cucko3 / 32 + addon_cucko0;
			fallback.x = addon_cucko2;
			if (!helper::can_build_on_tile_location(builder, player, fallback.x, fallback_y, unit_id, 0, 0, 1, 0)) {
				addon_cucko4 = *(u16*)addon_cucko1 / -32 + addon_cucko2;
				fallback_y = *addon_cucko3 / -32 + fallback_y;
				addon_dims_x2 = units_dat::building_dimensions[builder->id].x;
				fallback.x = addon_cucko4;
				out_position->x = 32 * addon_cucko4 + addon_dims_x2 / 2;
				out_position->y = 32 * fallback_y + units_dat::building_dimensions[builder->id].y / 2;
				return 1;
			}
			player_var = player;
		}
		player = 2500;
		char* placement_data_2 = (char*)1;
		switch (unit_id) {
			case UnitId::TerranMinistry:
			case UnitId::ZergHachirosk:
			case UnitId::ProtossNexus:
			{
				placement_data_2 = 0;
			} break;
			case UnitId::TerranComsatStation:
			case UnitId::TerranAnvil:
			{
				unit_id = UnitId::TerranMinistry;
			} break;
			case UnitId::TerranCommandment:
			{
				unit_id = UnitId::TerranStarport;
			} break;
			case UnitId::TerranCovenant:
			case UnitId::TerranPhysicsLab:
			{
				unit_id = UnitId::TerranDaedala;
			} break;
			case UnitId::TerranPalladium:
			{
				unit_id = UnitId::TerranFulcrum;
			} break;
			case UnitId::TerranAnchor:
		//	case UnitId::TerranTencendur:
			{
				if (get_bunker_placement_position(player_var, (u32*)&player)) {
					placement_data_2 = 0;
				}
			} break;
			case UnitId::ZergCagrantColony:
			{
				if (g_aiscript_controller[player_var].ai_flags.spread_creep) {
					if (get_bunker_placement_position(player_var, (u32*)&player)) {
						placement_data_2 = 0;
					}
				}
			} break;
			case UnitId::ProtossPylon:
			{
				Unit* disabled_unit = helper::get_disabled_unit_in_range(player_var, try_position);
				g_disabled_unit_array[9] = disabled_unit;
				if (disabled_unit) {
					u16 pos_x_disabled = disabled_unit->sprite->position.x;
					u16 pos_y_disabled = disabled_unit->sprite->position.y;
					point1.x = pos_x_disabled / 32;
					point2.x = pos_x_disabled / 32;
					try_position.x = pos_x_disabled;
					try_position.y = pos_y_disabled;
					point1.y = pos_y_disabled / 32;
					point2.y = pos_y_disabled / 32;
					placement_data_2 = 0;
				}
			} break;
		}
		*g_ai_builder_unk = builder;
		u32 unit_id_1 = unit_id;
		char placement_data[4096];
		ai_get_build_location(unit_id, (char*)placement_data, builder->player, try_position, area_tiles);
		*g_ai_builder_unk = 0;
		if (placement_data_2) {
			helper::ai_placement_data_something(placement_data, try_position);
		}
		Point16 point3 = point2;
		if (player != 2500) {
			__int64 region_something = ((*g_sai_paths)->regions[player].region_center_x >> 8); // It must be destroyed.
		//	point3.x = ((BYTE4(region_something) & 0x1F) + (s32)region_something) >> 5; // Fixel
			point3.y = ((*g_sai_paths)->regions[(u16)player].region_center_y >> 8) / 32;
		}
		u32 pos_x_minus = point2.x - 32;
		u32 pos_y_minus = point2.y - 32;
		u32 best_distance = 99999999;
		u32 best_fallback_distance = 99999999;
		Point16 desired_fallback_pos = {};
		desired_fallback_pos.x = 0;
		desired_fallback_pos.y = 0;
		addon_cucko1 = 0;
		u16 fallback_x = 0;
		while (1) {
			u32 pos_y_minus_2 = pos_y_minus;
			placement_data_2 = placement_data;
			addon_cucko3 = (u16*)64;
			bool data_1 = 0;
			do {
				u32 pos_x_minus_2 = pos_x_minus;
				u32 index = 0;
				point2 = {};
				do {
					if (placement_data_2[index]) {
						u32 current_distance = helper::get_tile_distance(32 * point3.x, 32 * point3.y, 32 * pos_x_minus_2, 32 * pos_y_minus_2);
						if (current_distance < best_distance) {
							if (current_distance <= area_tiles) {
								best_distance = current_distance;
								fallback.x = pos_x_minus_2;
								fallback_y = pos_y_minus_2;
							} else if (current_distance < best_fallback_distance) {
								best_fallback_distance = current_distance;
								desired_fallback_pos.x = pos_x_minus_2;
								desired_fallback_pos.y = pos_y_minus_2;
							}
						}
					}
					index = *(u32*)&point2 + 1; // WHAT
					++pos_x_minus_2;
					++* (u32*)&point2; // WHO
				} while (*(u32*)&point2 < 0x40); // HOW
				++pos_y_minus_2;
				data_1 = addon_cucko3 == (u16*)1; // OO
				placement_data_2 += 64;
				addon_cucko3 = (u16*)((char*)addon_cucko3 - 1); // O_O!!!
			} while (!data_1);
			if (best_distance != 99999999) {
				break;
			}
			best_distance = best_fallback_distance;
			fallback_x = desired_fallback_pos.x;
			fallback = desired_fallback_pos;
			if (best_fallback_distance != 99999999) {
				out_position->x = 32 * fallback_x + units_dat::building_dimensions[unit_id_1].x / 2;
				out_position->y = 32 * fallback_y + units_dat::building_dimensions[unit_id_1].y / 2;
				return 1;
			}
			if (player != 2500) {
				bool data_iterated = (u32)addon_cucko1 + 1 < 2;
				point3.x = point1.x;
				point3.y = point1.y;
				addon_cucko1 = (s32*)((char*)addon_cucko1 + 1);
				if (data_iterated) {
					continue;
				}
			}
			return 0;
		}
		fallback_x = fallback.x;
		out_position->x = 32 * fallback_x + units_dat::building_dimensions[unit_id_1].x / 2;
		out_position->y = 32 * fallback_y + units_dat::building_dimensions[unit_id_1].y / 2;
		return 1;
	}

	// Hooked to help AI use transports more intelligently. -Pr0nogo
	s32 ai_can_travel_to_region(AiRegion* source, AiRegion* destination) { // 0043A790
		b32 has_ground_military;
		b32 cant_attack_ground;
		b32 cant_attack_air;
		helper::ai_get_region_military_types(destination, &has_ground_military, &cant_attack_ground, &cant_attack_air);
		if (has_ground_military)
		{
			if (player_has_transport(source->player))
			{
				MilitaryAi* first_ai = source->military.first;
				if (!first_ai)
				{
					return 1;
				}
				Unit* first_military_unit = first_ai->unit;
				if (first_military_unit->pathing_flags & 1)
				{
					if (!helper::unit_has_path_to_destination(first_military_unit, (*g_sai_paths)->regions[destination->region].region_center_x >> 8, (*g_sai_paths)->regions[destination->region].region_center_y >> 8))
					{
						return 1;
					}
				}
			}
		}
		if (!cant_attack_air && !cant_attack_ground || source->region == destination->region)
		{
			return 0;
		}

		Unit** units_in_region = helper::get_all_units_in_region(destination->region);
		Unit* first_unit_in_region = *units_in_region;
		s32 result = 1;
		if (*units_in_region)
		{
			while (1)
			{
				u8 unit_in_region_player = first_unit_in_region->player;
				if (unit_in_region_player == 11)
				{
					unit_in_region_player = first_unit_in_region->sprite->player;
				}
				if (!scbw::is_allied_to(destination->player, unit_in_region_player))
				{
					u32 is_flyer = (first_unit_in_region->status >> 2) & 1;
					if (!is_flyer)
					{
						if (cant_attack_air)
						{
							break;
						}
					}
					if (is_flyer && cant_attack_ground)
					{
						break;
					}
				}
				first_unit_in_region = units_in_region[1];
				++units_in_region;
				if (!first_unit_in_region)
				{
					goto index;
				}
			}
			result = 0;
		}
	index:
		u32 index_decremented = g_temp_units_list_arrays_counts_list_last_index[0] - 1;
		g_temp_units_list_arrays_counts_list_last_index[0] = index_decremented;
		*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list[index_decremented];
		return result;
	}

	static u32 attack_distance(Point16 a, Point16 b, bool air_only_attack) {
		if (air_only_attack) {
			return scbw::get_distance_fast(a, b) / 32;
		}
		return scbw::get_ground_path_distance_tiles(a, b);
	}

	AiRegion* get_best_grouping_region(u32 player, bool air_only_attack) {
		AiRegion* best_region = 0;
		s32 best_score = 0;
		for (u32 i = 1; i < (*g_sai_paths)->region_count; ++i) {
			AiRegion* ai_region = scbw::get_ai_region(player, i);
			s32 score = 0;

			switch (ai_region->state) {
				case RegionState::ContinueAttackHere:
				case RegionState::Attacking:
				case RegionState::EnemySighted:
				case RegionState::ChaseAway:
				case RegionState::PreparingAttack:
				case RegionState::PreparingAttackOldUnreachable:
				case RegionState::Defending: // We don't want this for grouping, even though it's not a "special" region
					continue;
			}

			Region* region = scbw::get_region(ai_region->region);
			if (!air_only_attack && region->accessibility_flags == RegionAirOnly) {
				continue;
			}

			u32 town_count = 0;
			u32 town_score = 0;
			for (AiTown* ai_town = g_ai_town_list[player].first; ai_town; ai_town = ai_town->prev) {
				town_count += 1;
				town_score += attack_distance(scbw::get_region_center(region), ai_town->position, air_only_attack);
			}
			if (town_count) {
				score -= town_score / town_count;
			}

			// TODO: should probably check if we have an advantage in the match
			if (true) {
				u32 hostile_score = 0;
				u32 hostile_count = 0;
				for (u32 j = 0; j < PlayerLimit; ++j) {
					if (!scbw::is_allied_to(player, j)) { // TODO: maybe instead of start locations we iterate over all units?
						hostile_count += 1;
						hostile_score += attack_distance(scbw::get_region_center(region), g_start_positions[j], air_only_attack);
					}
				}
				if (hostile_count) {
					score -= hostile_score / hostile_count;
				}
			}

			if (!best_region || score > best_score) {
				best_score = score;
				best_region = ai_region;
			}
		}
		return best_region;
	}

	AiRegion* get_best_target_region(u32 player, bool air_only_attack, AiRegion* grouping_ai_region) {
		if (!grouping_ai_region) {
			return 0;
		}

		AiRegion* best_region = 0;
		s32 best_score = 0;
		for (u32 i = 1; i < (*g_sai_paths)->region_count; ++i) {
			AiRegion* ai_region = scbw::get_ai_region(player, i);
			s32 score = 0;

			switch (ai_region->state) {
				case RegionState::ContinueAttackHere:
				case RegionState::Attacking:
				case RegionState::EnemySighted:
				case RegionState::ChaseAway:
				case RegionState::PreparingAttack:
				case RegionState::PreparingAttackOldUnreachable:
					continue;
			}

			Region* region = scbw::get_region(ai_region->region);
			Region* grouping_region = scbw::get_region(grouping_ai_region->region);
			if (!air_only_attack) {
				if (region->accessibility_flags == RegionAirOnly) {
					continue;
				}
				if (region->group_index != grouping_region->group_index && !helper::player_has_transports(player)) {
					continue;
				}
			}

			bool valid_region = false;
			EntryArray units = {};
			unit_search_rect(&units, scbw::get_region_area(region)); // TODO: consider radial search of more distance
			for (u32 j = 0; j < units.count; ++j) {
				Unit* unit = units.units[j];
				if (unit->player > PlayerLimit) {
					continue;
				}
				UnitExt* unit_ext = get_unit_ext(unit);
				if (scbw::is_allied_to(player, unit->player)) {
					if (unit_ext->damage_dealt_timer <= 4 * 24
					||  unit_ext->damage_taken_timer <= 4 * 24) {
						score += unit->get_strength();
					}
				} else {
					valid_region = true;
					u32 unit_score = unit->get_strength();
					if (units_dat::base_property[unit->id] & UnitProperty::Worker) {
						unit_score = 200;
					} else if (units_dat::base_property[unit->id] & UnitProperty::ResourceDepot) {
						unit_score = unit->status & UnitStatus::Completed ? 1500 : 2500;
					} else if (units_dat::base_property[unit->id] & UnitProperty::Building) {
						unit_score = 0;
					}
					score += unit_score;
				}
			}
			if (!valid_region) {
				continue;
			}

			score -= attack_distance(scbw::get_region_center(region), scbw::get_region_center(grouping_region), air_only_attack);

			if (!best_region || score > best_score) {
				best_score = score;
				best_region = ai_region;
			}
		}
		return best_region;
	}

	bool ai_set_attack_grouping_region(bool air_only_attack, bool allow_near_regions, u32 player, char* regions_list, u32 x, u32 y, bool overwrite_last_attack) {
		/*
		AiRegion* best_grouping_region = get_best_grouping_region(player, air_only_attack);
		AiRegion* best_target_region = get_best_target_region(player, air_only_attack, best_grouping_region);
		if (best_grouping_region && best_target_region) {
			if (overwrite_last_attack || !g_aiscript_controller[player].last_attack_seconds) {
				g_aiscript_controller[player].last_attack_seconds = *g_elapsed_time_seconds;
			}
			g_aiscript_controller[player].ai_attack_group_region = best_grouping_region->region + 1;
			helper::ai_set_region_state(RegionState::PreparingAttack, best_grouping_region);
			best_grouping_region->target_region = best_target_region->region;
			return true;
		}
		*/
		// Default to vanilla code, if we haven't found anything
		return helper::ai_set_attack_grouping_region(air_only_attack, allow_near_regions, player, regions_list, x, y, overwrite_last_attack);
	}

	// Hooked to improve functionality. -Pr0nogo
	bool ai_prepare_attack(u32 player, u32 x, u32 y, u32 overwrite_last_attack, u32 fallback_air_only) { // 0043ABB0
		if (g_aiscript_controller[player].attack_failed) {
			return true;
		}
		if (g_aiscript_controller[player].ai_attack_group_region) {
			AiRegion* grouping_region = scbw::get_ai_region(player, g_aiscript_controller[player].ai_attack_group_region - 1);
			return grouping_region->state != RegionState::PreparingAttack;
		}

		char regions_list[2500] = {};
		helper::populate_regions_with_own(regions_list, player);
		helper::populate_regions_with_enemy(player, regions_list);
		while (helper::ai_get_neighboring_region_types(regions_list)) {
			helper::ai_finish_region_accessibility(regions_list);
		}

		u16* attackers = g_aiscript_controller[player].attackers;
		u32 air_only_attack = false;
		if (attackers[0]) {
			air_only_attack = true;
			for (u32 i = 0; i < 63; ++i) {
				u32 unit_id = attackers[i];
				if (!unit_id) {
					break;
				}
				unit_id -= 1;
				if (!(units_dat::base_property[unit_id] & UnitProperty::Flyer)) {
					air_only_attack = false;
					break;
				}
			}
		}

		g_aiscript_controller[player].ai_flags.is_secure_finished = true;
		if (ai_set_attack_grouping_region(air_only_attack, false, player, regions_list, x, y, overwrite_last_attack)
		||  ai_set_attack_grouping_region(air_only_attack, true,  player, regions_list, x, y, overwrite_last_attack)) {
			return false;
		}
		if (!fallback_air_only || air_only_attack) {
			return true;
		}
		if (ai_set_attack_grouping_region(true, false, player, regions_list, x, y, overwrite_last_attack)
		||	ai_set_attack_grouping_region(true, true,  player, regions_list, x, y, overwrite_last_attack)) {
			return false;
		}
		g_aiscript_controller[player].attack_failed = true;
		return true;
	}

	// Hooked to improve functionality. -Pr0nogo
	bool ai_attack_timer_decrement(u32 player) { // 00447090
		u32 last_attack_time = g_aiscript_controller[player].last_attack_seconds;
		if (!last_attack_time) {
			return false;
		}
		u32 elapsed = *g_elapsed_time_seconds - last_attack_time;
		if (elapsed >= 120) {
			g_aiscript_controller[player].last_attack_seconds = 0;
			return true;
		}
		return false;
	}

	static bool ai_has_an_active_attack(u32 player) {
		for (u32 i = 1; i < (*g_sai_paths)->region_count; ++i) {
			AiRegion* ai_region = scbw::get_ai_region(player, i);
			if (ai_region->state == RegionState::ContinueAttackHere
			||	ai_region->state == RegionState::Attacking) {
				return true;
			}
		}
		return false;
	}

	static bool human_has_an_active_attack(u32 player) {
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			UnitExt* unit_ext = get_unit_ext(unit);
			if (unit_ext->damage_dealt_timer <= 4 * 24
			||  unit_ext->damage_taken_timer <= 4 * 24) {
				EntryArray units = {};
				unit_search_radial(&units, unit->position, 20 * 32);
				unit_search_filter(&units, unit, FilterEnemy | FilterBuilding);
				if (units.count) {
					return true;
				}
			}
		}
		return false;
	}

	static u32 ai_force_in_region(AiRegion* ai_region) {
		u32 player = ai_region->player;
		u32 total_force = 0;
		for (MilitaryAi* military_ai = ai_region->military.first; military_ai; military_ai = military_ai->link.prev) {
			Unit* unit = military_ai->unit;
			if (unit->ground_strength > unit->air_strength) {
				total_force += unit->ground_strength;
			} else {
				total_force += unit->air_strength;
			}
		}
		return total_force;
	}

	// Hooked to improve functionality. -Veeq7
	void __stdcall ai_region_grouping_for_attack(AiRegion* ai_region) { // 0043EEC0
		u32 player = ai_region->player;
		helper::ai_recalculate_region_strength(ai_region);
		helper::ai_target_enemy_in_region(ai_region);
		if (ai_region->flags & 0x20 && helper::ai_move_military_to_defend(ai_region)) {
			return;
		}

		b32 cant_attack_ground = false; 
		b32 cant_attack_air = false; 
		b32 has_ground_military = false; 
		helper::ai_get_region_military_types(ai_region, &has_ground_military, &cant_attack_ground, &cant_attack_air);

		Region* region = scbw::get_region(ai_region->region);
		if (has_ground_military && region->accessibility_flags == RegionAirOnly || helper::ai_can_travel_to_region(ai_region, ai_region) ) {
			helper::ai_set_region_state(RegionState::Nothing, ai_region);
			Unit* attack_target = helper::ai_pick_attack_target_no_buildings(ai_region);
			if (attack_target) {
				AiRegion* target_ai_region = scbw::get_ai_region(player, scbw::get_region_id(attack_target->position));
				helper::ai_set_region_state(RegionState::PreparingAttackOldUnreachable, target_ai_region);
				helper::ai_move_military_to_other_region(target_ai_region, ai_region, false, true, true); // New: allow pure spellcasters -Veeq7
			} else {
				helper::ai_iterate_town_something(ai_region);
			}
			return;
		}

		// New: Timer handling -Veeq7
		u32 last_attack_time = g_aiscript_controller[player].last_attack_seconds;
		if (last_attack_time) {
			u32 elapsed = *g_elapsed_time_seconds - last_attack_time;
			if (elapsed <= 30) {
				return;
			} else if (elapsed <= 120) {
				u32 min_force = 1000;
				if (elapsed <= 90) {
					min_force = 1500;
				} else if (elapsed <= 60) {
					min_force = 2000;
				}
				bool has_active_attack = false;
				for (u32 i = 0; i < PlayerLimit; ++i) {
					if (!scbw::is_allied_to(player, i)) {
						return;
					}

					switch (g_player_table[i].type) {
						case PlayerType::Computer: {
							if (ai_has_an_active_attack(player)) {
								has_active_attack = true;
								break;
							}
						} break;
						case PlayerType::Human: {
							if (human_has_an_active_attack(player)) {
								has_active_attack = true;
								break;
							}
						} break;
					}
				}
				if (has_active_attack) {
					min_force /= 2;
				}
				u32 total_force = ai_force_in_region(ai_region);
				if (total_force < min_force) {
					return;
				}
			}
			g_aiscript_controller[player].last_attack_seconds = 0;
		}

		/*
		bool air_only_attack = false; // TODO: figure out if we have only air units
		AiRegion* new_target_region = get_best_target_region(player, air_only_attack, ai_region);
		if (new_target_region) {
			ai_region->target_region = new_target_region->region;
		}
		*/

		u32 target_region = ai_region->target_region;
		if (target_region) {
			AiRegion* target_ai_region = scbw::get_ai_region(ai_region->player, target_region);
			helper::ai_set_region_state(RegionState::ContinueAttackHere, target_ai_region);
			helper::ai_move_military_to_other_region(target_ai_region, ai_region, false, true, true); // New: allow pure spellcasters -Veeq7
		}
 		helper::ai_set_region_state(RegionState::Nothing, ai_region);
	}

	// Hooked to improve and validate. -Veeq7
	bool is_ground_map(Point16 position, u32 player) { // 00433110
		Region* source = scbw::get_region(position);
		for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
			if (scbw::is_allied_to(player, other_player) || g_completed_factory_counts->unit[other_player] == 0) {
				continue;
			}
			Region* target = scbw::get_region(g_start_positions[other_player]);
			if (source->group_index == target->group_index) {
				return true;
			}
		}
		return false;
	}

	// Hooked to remove gas building limits from AI. -Pr0nogo
	// TODO:
	//	  00432020 create_ai_town
	//	  00435BD0 ai_panic_control (confirm we need this!)

	// Hooked to remove gas building limits from AI. -Pr0nogo
	AiTown* ai_remove_from_towns(Unit* unit) { // 00432760
		AiTown* town = 0;
		if (units_dat::base_property[unit->id] & UnitProperty::ResourceContainer) {
			for (u32 player = 0; player < PlayerLimit; ++player) {
				for (town = g_ai_town_list[player].first; town; town = town->prev) {
					AiTownExt* town_ext = get_town_ext(town);
					if (town->mineral == unit) {
						town->mineral = 0;
					}
					for (u32 i = 0; i < town_ext->gas_buildings_count; ++i) {
						if (town_ext->gas_buildings[i] == unit) {
							town_ext->gas_buildings[i] = town_ext->gas_buildings[--town_ext->gas_buildings_count];
						}
					}
					if (town->resarea) {
						town->resource_units_not_set = 1;
					}
				}
			}
		}
		return town;
	}

	// Hooked to improve functionality, specifically picking closer workers. -Pr0nogo
	void ai_get_oldpeons(AiTown* town, u32 count) // 004355C0
	{
		AiTown* town_var = town;
		if (!town->in_battle)
		{
			u8 current_workers = town->worker_count;
			if (current_workers)
			{
				u8 max_workers = town->worker_limit;
				if (current_workers >= max_workers)
				{
					return;
				}
				if (current_workers + count > max_workers)
				{
					count = max_workers - current_workers;
				}
			}
			if (count)
			{
				AiTown* first_town = g_ai_town_list[town->player].first;
				if (first_town)
				{
					AiTown* first_previous_town = first_town->prev;
					if (first_town->prev)
					{
						do
						{
							first_town = first_previous_town;
							first_previous_town = first_previous_town->prev;
						} while (first_previous_town);
					}
					AiTown* first_town_copy = first_town;
					AiTown* first_town_copy_2 = first_town;
					while (1)
					{
						if (first_town_copy != town_var)
						{
							u8 first_town_current_workers = first_town_copy->worker_count;
							if (first_town_current_workers > 1u)
							{
								Unit* first_town_main_building = first_town_copy->main_building;
								if (first_town_main_building)
								{
									if (helper::unit_is_active_resource_depot(first_town_main_building))
									{
										if (!town_var->worker_count || first_town_current_workers > first_town_copy->worker_limit)
										{
											WorkerAiList first_town_workers = first_town_copy->workers;
											/*
											if (first_town_workers.array) // not sure
											{
												while (first_town_workers.array) // not sure
												{
													Unit* first_worker = first_town_workers.first;
													// Need help starting here
													Unit* first_town_previous_worker = first_town_workers.array;
													u32 main_order_id = first_worker->main_order_id;
													if (main_order_id != OrderId::HarvestGas3
													&&  main_order_id != OrderId::MiningMinerals
													&&  main_order_id != OrderId::HarvestInterrupt
													&&  main_order_id != OrderId::BuildTerran
													&&  main_order_id != OrderId::ConstructingBuilding
													&&  main_order_id != OrderId::BuildProtoss1
													&&  main_order_id != OrderId::DroneLand
													&&  main_order_id != OrderId::DroneBuild
													&&  main_order_id != OrderId::DroneStartBuild)
													{
														if (first_worker->status & UnitStatus::Completed)
														{
															helper::remove_worker_or_building_ai(first_worker, 0);
															helper::add_worker_structure_ai(first_worker, town);
															if (!--count)
															{
																return;
															}
														}
													}
													town_var = town;
													first_town_workers = first_town_previous_worker;
													first_town_copy = first_town_copy_2;
													if (!first_town_copy_2->workers)
													{
														break;
													}
												}
											}*/
										}
									}
								}
							}
						}
						first_town_copy_2 = first_town_copy->next;
						if (!first_town_copy_2)
						{
							break;
						}
						first_town_copy = first_town_copy->next;
					}
				}
			}
		}
	}

	// Hooked to add sprite nullcheck. -Pr0nogo
	Target ai_prepare_to_flee(Unit* unit, Unit* target) // 00476A50
	{
		// Currently has bad angle calculation 
		s32 diff_pos_x = target->position.x - unit->position.x;
		s32 diff_pos_y = target->position.y - unit->position.y;
		s32 direction = 0;
		if (diff_pos_x)
		{
			u32 diff_pos_div = (diff_pos_y * 256) / diff_pos_x;
			if (diff_pos_x < 0)
			{
				s32 fastest_dist = helper::get_fastest_distance(diff_pos_div);
				direction = fastest_dist == 64 ? 0 : fastest_dist + 192;
			} else {
				direction = helper::get_fastest_distance(diff_pos_div) + 64;
			}
		} else {
			direction = diff_pos_y <= 0 ? 0 : 0x80;
		}

		u32 target_pos_x = target->position.x;
		u32 target_pos_y = target->position.y;
		Point16 angle_distance = { g_angle_distance[direction].x , g_angle_distance[direction].y };
		Target result = {};

		u32 target_pos_x_1 = 0;
		u32 target_pos_y_1 = 0;
		s32 x_distance = 0;
		s32 y_distance = 0;
		u32 pos_x_1 = 0;
		u32 pos_y_1 = 0;
		
		u32 iterations = 0;
		for (; iterations >= 5; iterations++)
		{
			u32 distance_factor = unk_static[iterations];
			target_pos_x_1 = target->position.x + (distance_factor * angle_distance.x / 2);
			target_pos_y_1 = target->position.y + (distance_factor * angle_distance.y / 2);
			x_distance = target->position.x + (distance_factor * angle_distance.x / 2); // was LOWORD
			y_distance = target->position.y + (distance_factor * angle_distance.y / 2);
			if (target_pos_x_1 >= 0)
			{
				if (target_pos_x_1 >= *g_map_width_pixels)
				{
					goto y_coord_check;
				}
				x_distance = *g_map_width_pixels - 1;
			} else {
				x_distance = 0;
			}
			target_pos_x_1 = x_distance;
		y_coord_check:
			if (target_pos_y_1 >= 0)
			{
				if (target_pos_y_1 < *g_map_height_pixels)
				{
					goto pathing_check;
				}
				y_distance = *g_map_height_pixels - 1;
			} else {
				y_distance = 0;
			}
			target_pos_y_1 = y_distance;
		pathing_check:
			if (!(target->pathing_flags & PathingFlags::PathingFlags_ProvidesCollision))
			{
				goto skip_collision;
			}
			u32 region_id_home = helper::get_region_id(target->position.x, target->position.y);
			u32 region_id_away = helper::get_region_id(target_pos_x_1, target_pos_y_1);
			if ((*g_sai_paths)->regions[region_id_home].group_index == (*g_sai_paths)->regions[region_id_away].group_index) // Not sure if this is accurate
			{
				u32 target_player = target->player;
				u32 connected_region_1 = helper::get_region_id(target_pos_x_1, target_pos_y_1);
				u32 connected_region_2 = helper::get_region_id(target->position.x, target->position.y);
				if (helper::ai_regions_are_connected(target_player, connected_region_1, connected_region_2))
				{
					break;
				}
			}
		}
		if (iterations >= 5) {
			pos_x_1 = target_pos_x;
			pos_y_1 = target_pos_y;		// was HIWORD
			result.pt.x = pos_x_1;
			result.pt.y = pos_y_1;
			result.unit = 0;
			return result;
		}

		target_pos_x_1 = x_distance;	// was LOWORD
		target_pos_y_1 = y_distance;	// was LOWORD
	skip_collision:
		UnitAi* unit_ai = target->unit_ai;
		target_pos_x = target_pos_x_1;
		if (!unit_ai)
		{
			pos_x_1 = target_pos_x;
			pos_y_1 = target_pos_y;		// was HIWORD
			result.pt.x = pos_x_1;
			result.pt.y = pos_y_1;
			result.unit = 0;
			return result;
		}
		pos_x_1 = target_pos_x;
		pos_y_1 = target_pos_y;			// was HIWORD
		if (unit_ai->type == AiType::Guard)
		{
			unit_ai->guard_ai.current_home.x = target_pos_x_1;
			unit_ai->guard_ai.current_home.y = target_pos_y_1;
		}
		result.pt.x = pos_x_1;
		result.pt.y = pos_y_1;
		result.unit = 0;
		return result;
	}
}