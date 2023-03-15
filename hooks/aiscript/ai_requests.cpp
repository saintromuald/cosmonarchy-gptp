#include "ai_requests.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"
#include "ai_layouts.h"

namespace hooks
{
	// Don't know what this was hooked for; tidied May 2021 - Pr0nogo
	void ai_build_tech_upgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need) // 00431F90
	{
		s32 iterate = 0;
		constexpr s32 BuildCountFlag = 0xF8;
		TownReq* town_units;
		if (g_player_table[(u8)town->player].type == PlayerType::Computer)
		{
			town_units = town->requests;
			bool setNeed = false;
			if (town->requests[0].flags_and_count & BuildCountFlag) // if build
			{
				for (s32 i = 0; i < 100; i++)
				{
					++town_units;
					if (!(town_units->flags_and_count & BuildCountFlag)) // if not build
					{
						setNeed = true;
						iterate = i;
						break;
					}
				}
			}
			else
			{
				setNeed = true;
			}
			if (setNeed)
			{
				town->requests[iterate].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->requests[iterate].id = id;
				town->requests[iterate].priority = priority;
				// build orders have 0xf8 flag, tech has 0x4, upgrade has 0x2, if needed (player_need) has 0x1
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	AiTown* ai_progress_build(s32 player, s32* minerals, s32* gas) // 00434480
	{
		AiTown* result = g_ai_town_list[player].first;
		AiTown* first_town = g_ai_town_list[player].first;
		if (result)
		{
			do
			{
				WorkerAi* first_worker = first_town->workers.first;
				if (first_town->workers.first)
				{
					do
					{
						Unit* worker = first_worker->parent; // This was null during a crash
						if (!worker)
						{
							scbw::print("null unit in AI_progressBuild");
							return result;
						}
						s32 build_order_id = (u8)worker->main_order_id;
						if (build_order_id == OrderId::BuildTerran
						  || build_order_id == OrderId::BuildProtoss1
						  || build_order_id == OrderId::DroneLand
						  || build_order_id == OrderId::DroneBuild
						  || build_order_id == OrderId::DroneStartBuild)
						{
							u8* console_var = nullptr;
							s32 player_var = (u8)player;
							u16 target_unit_queue = worker->build_queue[worker->build_queue_slot % 5];
							s32 target_unit_queue2 = target_unit_queue;
							s32 target_queue_mineral_cost = units_dat::mineral_cost[target_unit_queue2];
							s32 target_queue_gas_cost = units_dat::vespene_cost[target_unit_queue2];
							g_ai_minerals_reserve[player_var] = target_queue_mineral_cost;
							g_ai_vespene_reserve[player_var] = target_queue_gas_cost;
							if (helper::has_supplies_for_unit(target_unit_queue, player, 1))
							{
								if (g_game_minerals[(u8)player] >= (s32)g_ai_minerals_reserve[(u8)player])
								{
									if (g_game_vespene[(u8)player] >= (s32)g_ai_vespene_reserve[(u8)player])
									{
										*minerals -= g_ai_minerals_reserve[player];
										*gas -= g_ai_vespene_reserve[player];
									}
									else
									{
										console_var = g_console_index;
										helper::display_error_message_id_with_sfx((u8)player, 851, *console_var + 150);
									}
								}
								else
								{
									console_var = g_console_index;
									helper::display_error_message_id_with_sfx((u8)player, 850, *console_var + 147);
								}
							}
						}
						first_worker = first_worker->link.prev;
					}
					while (first_worker);
				}
				result = first_town->prev;
				first_town = first_town->prev;
			}
			while (first_town);
		}
		return result;
	}

	char ai_perform_task(Unit* unit) { // 004361A0 -- hooked to fix AI Archon training -- Pr0nogo
		switch (unit->id) {
			case UnitId::ZergOvileth:
			case UnitId::ZergIroleth:
			case UnitId::ZergOthstoleth:
			case UnitId::ZergAlaszileth:
			{
				helper::ai_overlord_behavior(unit);
			} break;
			case UnitId::TerranComsatStation:
			{
				helper::ai_cast_spell_behavior(unit, 0);
			} break;
			case UnitId::TerranMissileSilo:
			{
				helper::ai_nuke_silo_behavior(unit);
			} break;
		}

		UnitAi* unit_ai = unit->unit_ai;
		if (!unit_ai || unit_ai->type != AiType::Building) {
			return true;
		}

		u32 player = unit->player;
		bool recurse = false;
		if (g_game_minerals[player] > 1500 && g_game_vespene[player] > 1500) {
			if (!helper::unit_is_training_or_morphing(unit)) {
				if (helper::ai_unit_construction_behavior(player, unit, 160)) {
					recurse = true;
				}
			}
		}

		u32 request_type = RequestType::None;
		if (g_aiscript_controller[player].request_count) {
			request_type = g_aiscript_controller[player].requests[0].type;
		}

		switch (request_type) {
			case RequestType::None:
			{
				if (!helper::ai_unit_construction_behavior(player, unit, 50))
				{
					return true;
				}
				recurse = true;
				return helper::ai_plan_do_merge_morph(player, unit, recurse);
			}
			case RequestType::Military:
			case RequestType::Guard:
			{
				return helper::ai_plan_do_merge_morph(player, unit, recurse);
			}
			case RequestType::Building:
			{
				return helper::ai_morph_behavior(player, unit, unit->unit_ai);
			}
			case RequestType::Worker:
			{
				helper::ai_train_behavior(unit, unit->unit_ai, player);
				return true;
			} break;
			case RequestType::Upgrade:
			{
				helper::ai_upgrade_behavior(unit, player);
				return true;
			} break;
			case RequestType::Tech:
			{
				helper::ai_research_behavior(unit, player);
				return true;
			} break;
			case RequestType::Addon:
			{
				helper::ai_construct_behavior(unit, unit->unit_ai, player);
				return true;
			} break;
			case RequestType::Witness:
			{
				return helper::ai_make_detectors_behavior(unit, player);
			} break;
		}
		return true;
	}

	char ai_plan_do_merge_morph(s32 player, Unit* unit, s32 boolSomething) // 00435F10
	{
		Unit* unit_copy = unit;
		s32 next_build_type = g_aiscript_controller[player].requests->type;
		Unit* dummy = nullptr;
		switch (next_build_type)
		{
			case UnitId::ZergGeszithalor:
			case UnitId::ZergVithrilisk:
				unit_copy = helper::ai_get_mutalisk_for_morph(player);
				if (!unit_copy)
				{
					helper::ai_reserve(player, 0);
					helper::ai_pop_spending_request(player);
					return 0;
				}
				break;
			case UnitId::ProtossArchon:
				if (helper::ai_get_high_templar_for_summon(player, &unit, &dummy))
				{
					helper::assign_order_with_target(dummy, unit, OrderId::WarpingArchon);
					helper::assign_order_with_target(unit, dummy, OrderId::WarpingArchon);
					helper::ai_reserve(player, 1);
					helper::ai_pop_spending_request(player);
					return 0;
				}
			case UnitId::ProtossMindTyrant:
				if (helper::ai_get_dark_templar_for_summon(player, &dummy, &unit))
				{
					helper::assign_order_with_target(dummy, unit, OrderId::WarpingDarkArchon);
					helper::assign_order_with_target(unit, dummy, OrderId::WarpingDarkArchon);
					helper::ai_reserve(player, 1);
					helper::ai_pop_spending_request(player);
					return 0;
				}
				helper::ai_reserve(player, 0);
				helper::ai_pop_spending_request(player);
				return 0;
			case UnitId::ZergLakizilisk:
				unit_copy = helper::AI_GetHydraliskForMorph(player);
				if (!unit_copy)
				{
					helper::ai_reserve(player, 0);
					helper::ai_pop_spending_request(player);
					return 0;
				}
				break;
		}

		if (!helper::is_unit_id_race_equal_to_unit_race(next_build_type, unit_copy) || helper::create_unit_allowed(next_build_type, unit_copy, player) != 1)
		{
			if (*g_last_internal_error == 2
			  || *g_last_internal_error == 23
			  || *g_last_internal_error == 21
			  || *g_last_internal_error == 8
			  || *g_last_internal_error == 7)
			{
				helper::ai_reserve(player, 0);
				helper::ai_pop_spending_request(player);
				if (!boolSomething)
				{
					helper::ai_perform_task(unit);
				}
				return 0;
			}
			if (*g_last_internal_error == 13)
			{
				helper::ai_reserve(player, 0);
				helper::ai_pop_spending_request(player);
				if (!boolSomething)
				{
					helper::ai_perform_task(unit);
				}
				return 0;
			}
		}

		if (!helper::has_resources_for_unit(next_build_type, player))
		{
			if ((g_aiscript_controller[player].ai_flags.is_use_map_settings) == 0)
			{
				helper::ai_reserve(player, 0);
				helper::ai_pop_spending_request(player);
				if (!boolSomething)
				{
					helper::ai_perform_task(unit);
				}
			}
			return 0;
		}

		if (helper::ai_isnt_at_define_max(player, next_build_type))
		{
			helper::ai_reserve(player, 0);
			helper::ai_pop_spending_request(player);
			return 0;
		}

		if (unit == unit_copy)
		{
			u32 requested_build_id = helper::ai_get_request_build_type(player);
			if (!helper::ai_train_unit(unit_copy, next_build_type, requested_build_id, g_aiscript_controller[player].requests->value))
			{
				return 0;
			}
		}
		else
		{
			if (!helper::has_money_can_make(unit_copy, next_build_type))
			{
				return 0;
			}
			helper::remove_unit_ai(unit_copy, 0);
			if (helper::ai_get_request_build_type(player) == 2)
			{
				/*
				BuildingAi* unitAI = (BuildingAi*)g_aiscript_controller[player].requests->val;
				mainTownTown = unitAI->town;
				unitAIQueues = (s16)unitAI->trainQueueValues;
				unitAI->parent = unit_copy;
				unitAI->region = mainTownTown;
				unitAI->town = unitAIQueues;
				unit_copy->pAI = unitAI;
				*/
			}
			else
			{
				helper::add_military_ai((AiRegion*)g_aiscript_controller[player].requests->value, unit_copy, 1);
			}
		}

		helper::ai_reserve(player, 1);
		helper::ai_pop_spending_request(player);
		if (scbw::is_conventional_unit_morph_source(unit_copy->id))
		{
			helper::issue_order_targeting_nothing(unit_copy, OrderId::Morph1);
			return 1;
		}
		else
		{
			helper::set_secondary_order(unit_copy, OrderId::Train);
			return 1;
		}
	}

	// Hooked to add more transport calls and hopefully improve AI defense logic. -Pr0nogo
	void ai_train_defensive_military(u8 player_id) // 0043F990
	{
		u32 region_count;
		s32 v2;
		u8 v3;
		AiRegion* player_regions;
		AiRegion* military_region;
		u16 defense_region_id;
		u32 i;
		AiRegion* defense_region;
		u32 region_air_strength;
		s32 defense_region_id_copy;
		u32 j;
		u32 region_gnd_strength;
		u16 ai_flags;
		u8 v15;
		bool finished_with_regions;
		s16 transport_unit_id_1;
		u32 witness_count;
		s32 witness_request_quantity;
		u8 witness_supply_cost;
		s32 witness_gas_cost;
		u32 transport_1_count;
		u32 transport_1_quantity;
		s32 transport_1_priority;
		u8 transport_supply_cost;
		s32 v29;
		s32 transport_1_gas_cost;
		s32 region_count_zeroed;
		u32 transport_1_request_count;
		AiRegion* v33;
		s32 v34;
		u16 transport_unit_id_2;

		region_count = (*g_sai_paths)->region_count;
		v33 = 0;
		if (region_count > 1)
		{
			v2 = 52;
			v34 = 52;
			region_count_zeroed = region_count - 1;
			do
			{
				player_regions = g_ai_region_captains[player_id];
				v3 = *(&player_regions->state + v2);
				military_region = (player_regions + v2);
				if (v3 > 3u)
				{
					if (v3 == 8)
					{
						helper::ai_region_train_attack_force(military_region);
					}
					else
					{
						if (v3 == 5 || v3 == 4)
						{
							v33 = military_region;
						}
						if (v3 < 7u)
						{
							defense_region_id = military_region->region;
						}
						else
						{
							defense_region_id = military_region->target_region;
						}
						region_air_strength = military_region->air_strength_full;
						defense_region_id_copy = defense_region_id;
						i = player_regions[defense_region_id_copy].air_strength_needed;
						defense_region = &player_regions[defense_region_id_copy];
						if (i)
						{
							for (i = i; i > region_air_strength; i = defense_region->air_strength_needed)
							{
								region_air_strength += helper::ai_defend_region(military_region, defense_region_id, 1, i - region_air_strength);
							}
						}
						j = defense_region->ground_strength_needed;
						region_gnd_strength = military_region->ground_strength_full;
						if (j)
						{
							for (j = j; j > region_gnd_strength; j = defense_region->ground_strength_needed)
							{
								region_gnd_strength += helper::ai_defend_region(military_region, defense_region_id, defense_region->air_strength_enemy != 0, j - region_gnd_strength);
							}
						}
						if (g_aiscript_controller[player_id].ai_flags.flag_0x40)
						{
							v15 = g_aiscript_controller[player_id].defense_priority_base;
							if (v15)
							{
								g_aiscript_controller[player_id].defense_priority_base = v15 - 1;
							}
							g_aiscript_controller[player_id].ai_flags.flag_0x40 = false;
						}
					}
				}
				v2 = v34 + 52;
				finished_with_regions = region_count_zeroed == 1;
				v34 += 52;
				--region_count_zeroed;
			} while (!finished_with_regions);
		}
		if (!(g_aiscript_controller[player_id].ai_flags.is_transports_off))
		{
			u32 player_race = g_player_table[player_id].race;
			if (player_race == RaceId::Terran)
			{
				if (!g_completed_unit_count_table[UnitId::TerranPhalanxTankModeTurret].player[player_id + 12]) // what
				{
					return;
				}
				transport_unit_id_1 = UnitId::TerranTrojan;
				transport_unit_id_2 = UnitId::TerranTrojan;
			}
			else
			{
				if (player_race != RaceId::Protoss)
				{
					return;
				}
				witness_count = g_all_unit_count_table[UnitId::ProtossWitness].player[player_id];
				if (witness_count < 2)
				{
					witness_request_quantity = 2 - witness_count;
					do
					{
						witness_gas_cost = units_dat::vespene_cost[UnitId::ProtossWitness];
						witness_supply_cost = units_dat::supply_required[UnitId::ProtossWitness];
						if (units_dat::base_property[UnitId::ProtossWitness] & UnitProperty::TwoUnitsIn1Egg)
						{
							witness_supply_cost = 2 * units_dat::supply_required[UnitId::ProtossWitness];
						}
						g_aiscript_controller[player_id].minerals_needed += units_dat::mineral_cost[UnitId::ProtossWitness];
						g_aiscript_controller[player_id].vespene_needed += witness_gas_cost;
						g_aiscript_controller[player_id].supply_needed += witness_supply_cost;
						helper::ai_add_spending_request(UnitId::ProtossWitness, player_id, 8, 80, 0);
						--witness_request_quantity;
					} while (witness_request_quantity);
				}
				transport_unit_id_2 = UnitId::ProtossEnvoy;
				transport_unit_id_1 = UnitId::ProtossEnvoy;
			}
			if (v33)
			{
				transport_1_count = g_all_unit_count_table[0].player[player_id + 12 * transport_unit_id_1];
				transport_1_quantity = (g_aiscript_controller[player_id].ai_flags.is_use_map_settings) != 0 ? 2 : 5;
				transport_1_priority = g_all_unit_count_table[0].player[player_id + 12 * transport_unit_id_1] != 0 ? 50 : 80;
				if (transport_1_count < transport_1_quantity)
				{
					transport_1_request_count = transport_1_quantity - transport_1_count;
					while (1)
					{
						g_aiscript_controller[player_id].ai_flags.flag_0x40 = true;
						transport_1_gas_cost = units_dat::vespene_cost[transport_unit_id_1];
						transport_supply_cost = units_dat::supply_required[transport_unit_id_1];
						if (units_dat::base_property[transport_unit_id_1] & UnitProperty::TwoUnitsIn1Egg)
						{
							transport_supply_cost *= 2;
						}
						v29 = g_aiscript_controller[player_id].vespene_needed;
						g_aiscript_controller[player_id].minerals_needed += units_dat::mineral_cost[transport_unit_id_1];
						g_aiscript_controller[player_id].vespene_needed = transport_1_gas_cost + v29;
						g_aiscript_controller[player_id].supply_needed += transport_supply_cost;
						helper::ai_add_spending_request(transport_unit_id_1, player_id, 1, transport_1_priority, v33);
						helper::ai_add_base_unit_spending_request(transport_unit_id_2, player_id, v33, transport_1_priority);
						if (!--transport_1_request_count)
						{
							break;
						}
						transport_unit_id_1 = transport_unit_id_2;
					}
				}
			}
		}
	}
	
	void ai_build(u32 unit_id, bool need, u32 count, AiTown* town, u8 priority) // 004323D0
	{
		if (count <= 30 && unit_id != UnitId::None)
		{
			helper::ai_add_build_request(town, 0, count, unit_id, priority, need);
		}
	}

	// Hooked to improve logic. -Pr0nogo
	char ai_build_refinery(AiTown* town) // 00433660
	{
		char result = 0;
		u32 player = town->player;
		u8 mineral_percent;
		u8 gas_percent;
		helper::ai_resource_need_split(&gas_percent, player, &mineral_percent);
		if (gas_percent || town->mineral)
		{
			Unit* refinery;
			u32 priority;
			if (helper::get_ai_refinery(town, 3, 1))
			{
				priority = 80;
				refinery = helper::get_ai_refinery(town, 1, 1);
				if (!refinery)
				{
					return 0;
				}

				u8 worker_count = town->worker_count;
				if (worker_count < town->worker_limit && worker_count < 0xFu)
				{
					return worker_count;
				}

				if (g_game_minerals[player] < 500)
				{
					return player;
				}
			}
			else
			{
				refinery = helper::get_ai_refinery(town, 1, 1);
				if (!refinery)
				{
					return 0;
				}
				priority = 90;
			}
			u8 race = g_player_table[player].race;
			// TODO: Add handling for custom races. -Pr0nogo
			u16 refinery_id = UnitId::TerranReservoir;
			if (race == RaceId::Zerg)
			{
				refinery_id = UnitId::ZergExcisant;
			}
			else if (race == RaceId::Protoss)
			{
				refinery_id = UnitId::ProtossAquifer;
			}
			result = helper::ai_add_town_spending_request(player, priority, refinery_id, town);
		}
		return result;
	}

	// Hooked to allow AI to train guards while under attack. -Pr0nogo
	void ai_request_guard(GuardAi* guard_ai, u32 player_id, u32 priority) { // 00448630
		u32 result = 0;
		u32 last_hit = g_aiscript_controller[player_id].previous_building_hit_second;
		/*
		if (last_hit && *g_elapsed_time_seconds - last_hit < 30) {
			return;
		}
		*/
		s32 unit_id = guard_ai->unit_id;
		if (unit_id == UnitId::TerranPhalanxSiegeMode) {
			unit_id = UnitId::TerranPhalanxTankMode;
		}
		helper::ai_add_cost_to_needs(unit_id, player_id, 1);
		helper::ai_add_spending_request(unit_id, player_id, 2, priority, guard_ai);
		helper::ai_add_base_unit_spending_request(unit_id, player_id, scbw::get_ai_region(player_id, scbw::get_region_id(guard_ai->current_home)), priority);
	}

	static bool is_valid_attack_unit(AiRegion* ai_region, Unit* unit, bool strict) {
		if (!(unit->status & UnitStatus::Completed) || !helper::is_valid_for(unit->id, unit, 0)) {
			return false;
		}

		if (!unit->unit_ai || (unit->unit_ai->type != AiType::Guard && unit->unit_ai->type != AiType::Military)) {
			return false;
		}

		if (strict) {
			// Skip transports
			if (scbw::unit_id_is_transport(unit->id)) {
				return false;
			}

			// Skip guards
			if (unit->unit_ai->type == AiType::Guard) {
				return false;
			}
		}

		if (unit->unit_ai->type == AiType::Military) {
			// Skip current attack force units
			AiRegion* current_ai_region = unit->unit_ai->military_ai.region;
			if (scbw::is_attack_region(current_ai_region)) {
				return false;
			}

			if (strict) {
				// Skip units from regions that don't have strength needed defined, since they are hard to assess properly
				// We will instead let AI assign units to defense regions and then pull the units to attack waves
				if (!current_ai_region->ground_strength_needed && !current_ai_region->air_strength_needed) {
					return false;
				}
			}

			// Skip units needed for defending regions, we take a portion of the needed force of less important regions,
			// and only excess force of the important regions
			u16 max_force = current_ai_region->defense_priority > 110 ? 0xffff : 100 * (*g_elapsed_time_frames / 24 / 60);
			if ((unit->ground_strength && current_ai_region->ground_strength_full - (s32)unit->ground_strength < std::min(current_ai_region->ground_strength_needed, max_force)) ||
				(unit->air_strength    && current_ai_region->air_strength_full    - (s32)unit->air_strength    < std::min(current_ai_region->air_strength_needed,    max_force))) {
				return false;
			}
		}

		// Skip units that can't access the region
		Region* region = scbw::get_region(ai_region->region);
		if (region->accessibility_flags == RegionAirOnly && unit->pathing_flags & 1) {
			return false;
		}

		return true;
	}

	static Unit* add_attack_unit(AiRegion* ai_region, u32 unit_id) {
		u32 player = ai_region->player;
		Region* region = scbw::get_region(ai_region->region);
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (unit->id != unit_id || !is_valid_attack_unit(ai_region, unit, false)) {
				continue;
			}
			if (scbw::move_unit_ai(unit, ai_region)) {
				return unit;
			}
		}
		return 0;
	}

	// Hooked to change attack_add priorities. -Pr0nogo
	void ai_region_train_attack_force(AiRegion* ai_region) { // 0043E670
		u32 attack_wave[63] = {};
		u32 priority = 90;
		u32 count = helper::get_missing_attack_force_units(ai_region, attack_wave, false); // Last arg used to be g_aiscript_controller[region->player].ai_flags.is_use_map_settings. We use melee for now, but we should invesitage it. -Veeq7
		for (u32 i = 0; i < count; ++i) {
			u32 unit_id = attack_wave[i];
			if (unit_id != UnitId::None && !add_attack_unit(ai_region, unit_id)) { // used to call !helper::ai_add_to_region_military(ai_region, unit_id, priority), now calling a custom function
				g_aiscript_controller[ai_region->player].ai_flags.flag_0x40 = true;
				helper::ai_add_cost_to_needs(unit_id, ai_region->player, 1);
				helper::ai_add_spending_request(unit_id, ai_region->player, 1, priority, ai_region);
				helper::ai_add_base_unit_spending_request(unit_id, ai_region->player, ai_region, priority);
			}
		}

		u32 player = ai_region->player;
		Region* region = scbw::get_region(ai_region->region);
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (!is_valid_attack_unit(ai_region, unit, true)) {
				continue;
			}
			scbw::move_unit_ai(unit, ai_region);
		}
	}

	// Hooked to allow AI to build while under attack. -Pr0nogo
	void ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, AiRegion* region) { // 00448400
		u32 last_hit = g_aiscript_controller[player].previous_building_hit_second;
		u32 race = g_player_table[player].race;
		/*
		if (last_hit && *g_elapsed_time_seconds - last_hit < 30) {
			return;
		}
		*/
		if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
			u32 type = 4;
			helper::ai_add_cost_to_needs(unit_id, player, type);
			helper::ai_add_spending_request(unit_id, player, type, priority, region);
		} else {
			u32 type = 2 * (units_dat::base_property[unit_id] & UnitProperty::Addon) | 3;
			helper::ai_add_cost_to_needs(unit_id, player, type);
			helper::ai_add_spending_request(unit_id, player, type, priority, region);
		}
	}

	static Unit* find_unit_for_guard(u32 player, u32 unit_id, Point16 position) {
		Unit* best_unit = 0;
		f32 best_score = 0;
		for (Unit* unit = first_player_unit->unit[player]; unit; unit = unit->player_link.next) {
			if (unit->id != unit_id) {
				continue;
			}

			if (!(unit->status & UnitStatus::Completed) || !helper::is_valid_for(unit->id, unit, 0)) {
				continue;
			}
			
			if (!unit->unit_ai || unit->unit_ai->type != AiType::Military) {
				continue;
			}

			AiRegion* current_ai_region = unit->unit_ai->military_ai.region;
			if (scbw::is_attack_region(current_ai_region)) {
				continue;
			}
			if (current_ai_region->defense_priority > 110) {
				continue;
			}
			
			f32 force = unit->ground_strength + unit->air_strength;
			if (!force) {
				continue;
			}

			f32 distance = helper::get_path_distance_to(unit, position);
			f32 score = distance >= 1 ? force / distance : force;
			if (score > best_score) {
				best_score = score;
				best_unit = unit;
			}
		}

		return best_unit;
	}

	// Hooked to improve AI guard handling. -Pr0nogo
	void ai_update_guard_needs(u32 player) { // 004630C0
		GuardAi* prev = 0;
		for (GuardAi* guard_ai = g_player_guard_list->player[player].first; guard_ai; guard_ai = prev) {
			prev = guard_ai->link.prev;
			if (guard_ai->times_died >= 100) {
				helper::delete_guard_ai(guard_ai, &g_player_guard_list->player[player]);
			} else if (guard_ai->parent) {
				Unit* unit = guard_ai->parent;
				guard_ai->current_home = guard_ai->original_home;
				if (unit->status & UnitStatus::IsHallucination) {
					guard_ai->parent = 0;
					helper::remove_unit_ai(unit, false);
					guard_ai->times_died -= 1;
					helper::ai_training_simple_military(unit, unit);
				}
			} else {
				AiRegion* ai_region = scbw::get_ai_region(player, scbw::get_region_id(guard_ai->original_home));
				Unit* unit = find_unit_for_guard(player, guard_ai->unit_id, guard_ai->original_home);
				if (unit) {
					helper::remove_unit_ai(unit, 0);
					guard_ai->current_home = guard_ai->original_home;
					guard_ai->parent = unit;
					unit->unit_ai = (UnitAi*)guard_ai;
					helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
				}
				// Request handling is currently in Aise, so we don't duplicate the priority system
			//	else if (!guard_ai->previous_update || *g_elapsed_time_seconds > guard_ai->previous_update + 5) {
			//		helper::ai_request_guard(guard_ai, player);
			//		guard_ai->previous_update = *g_elapsed_time_seconds;
			//	}
			}
		}
	}

	// Hooked to fix player_need and improve functionality. -Pr0nogo
	void ai_add_spending_request_from_towns(u32 player) { // 00436320
		g_aiscript_controller[player].ai_flags.upgrades_finished = true; // HIBYTE(g_aiscript_controller[player].ai_flags) |= 4u;

		u32 race = g_player_table[player].race;
		u32 builder_id = UnitId::None;
		u32 main_building_id = UnitId::None;
		// TODO: Add support for custom races. -Pr0nogo
		switch (race) {
			case RaceId::Terran: {
				main_building_id = UnitId::TerranCommandCenter;
				builder_id = UnitId::TerranMason;
			} break;
			case RaceId::Protoss: {
				main_building_id = UnitId::ProtossNexus;
				builder_id = UnitId::ProtossScribe;
			} break;
			case RaceId::Zerg: {
				main_building_id = UnitId::ZergHachirosk;
				builder_id = UnitId::ZergDroleth;
			} break;
		}

		s32 has_excess_workers = false;
		AiTown* town = 0;
		for (town = g_ai_town_list[player].first; town; town = town->prev) {
			helper::cancel_queued_trains(town);
			if (town->workers.first) {
				if (town->worker_count > town->worker_limit) {
					has_excess_workers = true;
				}
			} else {
				Unit* main_building = town->main_building;
				if (!main_building || main_building->status & UnitStatus::Completed || main_building->id != UnitId::ZergHachirosk) {
					helper::ai_get_old_peons(town, 1);
				}
			}

			if (town->in_battle) {
				continue;
			}

			for (u32 i = 0; i < ARRAY_SIZE(town->requests); ++i) {
				TownReq* request = &town->requests[i];

				if (!request->count) {
					break;
				}

				u32 existing_count = 0;
				if (request->is_upgrade) {
					existing_count = helper::get_upgrade_count(town, request->id);
				} else if (request->is_tech) {
					existing_count = helper::get_tech_count(town, request->id);
				} else if (request->only_if_needed) {
					existing_count = player_count_up_equivalents(player, request->id);
				} else {
					existing_count = town_count_up_equivalents(town, request->id);
				}

				u32 requested_count = request->count;
				if (request->id == builder_id && requested_count > town->worker_limit) {
					requested_count = town->worker_limit; // This is not supporting multiple worker ids properly (would have to count all worker ids in the town)
				}

				if (existing_count < requested_count) {
					u32 count = requested_count - existing_count;
					if (request->is_upgrade) {
						u32 mineral_cost = upgrades_dat::mineral_cost_base[request->id] + upgrades_dat::mineral_cost_factor[request->id] * scbw::get_upgrade_level(player, request->id);
						u32 vespene_cost = upgrades_dat::mineral_cost_base[request->id] + upgrades_dat::vespene_cost_factor[request->id] * scbw::get_upgrade_level(player, request->id);
						g_aiscript_controller[player].minerals_needed += mineral_cost * count;
						g_aiscript_controller[player].vespene_needed  += vespene_cost * count;
						for (u32 i = 0; i < count; ++i) {
							helper::ai_add_spending_request(request->id, player, RequestType::Upgrade, request->priority, town);
						}
					} else if (request->is_tech) {
						u32 mineral_cost = techdata_dat::mineral_cost[request->id];
						u32 vespene_cost = techdata_dat::vespene_cost[request->id];
						g_aiscript_controller[player].minerals_needed += mineral_cost * count;
						g_aiscript_controller[player].vespene_needed  += vespene_cost * count;
						for (u32 i = 0; i < count; ++i) {
							helper::ai_add_spending_request(request->id, player, RequestType::Tech, request->priority, town);
						}
					} else {
						bool request_base = request->only_if_needed ? false : true;
						scbw::town_add_spending_request(town, count, request->id, request->priority, request_base);
					}
				}
			}

			// Requesting refineries is currently handled in custom_ai
		//	if (town->worker_count > 1 || (town->worker_count == 1 && town->main_building && race == RaceId::Protoss)) {
		//		hooks::ai_build_refinery(town);
		//	}
		}

		// Last town handling
		if (town) {
			if (has_excess_workers) {
				for (AiTown* town = g_ai_town_list[player].first; town; town = town->prev) {
					Unit* main_building = town->main_building;
					if (main_building && main_building->status & UnitStatus::Completed) {
						if (town->worker_count < town->worker_limit) {
							helper::ai_get_old_peons(town, 1u);
						}
					}
				}
			}
			if (town) {
				// Originally called ai_build_supplies
				helper::ai_build_bunker(town, player);
				helper::ai_build_turret(town, player);
			}
			return;
		}
	}

	// Hooked to improve ordering consistency of spending requests. -Veeq7
	void pop_spending_request(u32 player) {	// 004476D0
		AI_Main* player_ai = &g_aiscript_controller[player];

		player_ai->request_count -= 1;
		player_ai->requests[0] = player_ai->requests[player_ai->request_count];
		player_ai->requests[player_ai->request_count] = {};
		if (player_ai->request_count <= 1) {
			return;
		}

		u32 index = 0;
		while (2 * index + 1 < player_ai->request_count) {
			u32 left_index  = 2 * index + 1;
			u32 right_index = 2 * index + 2;

			u32 highest_index = left_index;
			if (right_index < player_ai->request_count && player_ai->requests[right_index].priority > player_ai->requests[left_index].priority) {
				highest_index = right_index;
			}

			SpendingRequest request         = player_ai->requests[index];
			SpendingRequest highest_request = player_ai->requests[highest_index];
			if (highest_request.priority >= request.priority) {
				break;
			}
			player_ai->requests[index] = highest_request;
			player_ai->requests[highest_index] = request;
		}
	}

	u8 ai_get_build_order_id(Unit* unit, u32 unit_id) { // 00434C31
		u32 worker_race = unit->get_race();
		if (worker_race == Unit::get_race(unit_id)) {
			switch (worker_race) {
				case RaceId::Terran: {
					return OrderId::BuildTerran;
				} break;
				case RaceId::Protoss: {
					return OrderId::BuildProtoss1;
				} break;
				case RaceId::Zerg: {
					return OrderId::DroneStartBuild;
				} break;
			}
		}
		return OrderId::None;
	}

	// Hooked to make AI use only one worker per build request. -Pr0nogo
	s32 ai_try_progress_spending_queue_worker(AiTown* town, Unit* unit) { // 004348C0
		assert(town->player == unit->player);

		u32 player = unit->player;
		if (!g_aiscript_controller[player].request_count || g_aiscript_controller[player].requests[0].type != RequestType::Building) {
			return false;
		}

		UnitAi* unit_ai = unit->unit_ai;
		if (g_aiscript_controller[player].requests[0].value != unit_ai->worker_ai.town) {
			return false;
		}

		// Originally counted a per-worker variable that prohibited that worker from attempting construction.

		u32 unit_id = g_aiscript_controller[player].requests[0].id;
		if (!helper::is_unit_id_race_equal_to_unit_race(unit_id, unit)
		||   helper::create_unit_allowed(unit_id, unit, player) != 1
		||  !helper::has_resources_for_unit(unit_id, player)) {
			return false;
		}

		Point16 build_pos = {};
		if (unit_id == helper::get_default_refinery_type(player)) {
			Unit* gas = helper::get_ai_refinery(town, 1, 1);
			if (!gas) {
				return false;
			}
			build_pos = gas->position;
		} else {
			switch (unit_id) {
				case UnitId::TerranBunker:
				case UnitId::PoliceDugout: {
					if (!helper::get_bunker_position(player, &build_pos)) {
						build_pos = town->position;
					}

					u32 region_id = helper::get_region_id(build_pos.x, build_pos.y);
					helper::do_area_fixups_for_enemy(player, region_id, 1);
					if (g_aiscript_controller[player].failed_placement) {
						return helper::ai_finish_unit(unit);
					}

					if (!helper::ai_manage_building_placement(unit, unit_id, build_pos, &build_pos, 64)) {
						g_aiscript_controller[player].failed_placement = true;
						return helper::ai_finish_unit(unit);
					}

					region_id = scbw::get_region_id(build_pos);
					AiRegion* ai_region = scbw::get_ai_region(player, region_id);
					if (!ai_region->state) {
						helper::ai_set_region_state(4, ai_region);
						ai_region->flags |= 0x1;
						ai_region->flags |= 0x40;

						Region* region = scbw::get_region(region_id);
						if (region->accessibility_flags != RegionAirOnly) {
							ai_region->ground_strength_needed = 1000;
						}
					}
				} break;
				case UnitId::TerranWatchdog: {
					if (!helper::get_turret_position(player, &build_pos)) {
						build_pos = town->position;
					}
					if (g_aiscript_controller[player].failed_placement) {
						return helper::ai_finish_unit(unit);
					}
					if (!helper::ai_manage_building_placement(unit, UnitId::TerranWatchdog, build_pos, &build_pos, 40)) {
						g_aiscript_controller[player].failed_placement = true;
						return helper::ai_finish_unit(unit);
					}
				} break;
				default: {
					u32 building_placement = 0;
					if (unit_id == UnitId::ZergHachirosk && town->main_building) {
						helper::get_secondary_hachirosk_build_location(town->resarea, &build_pos.x, &build_pos.y);
						if (g_aiscript_controller[player].failed_placement) {
							return helper::ai_finish_unit(unit);
						}
						building_placement = helper::ai_manage_building_placement(unit, UnitId::ZergHachirosk, build_pos, &build_pos, 64);
					} else {
						build_pos = town->position;
						if (unit_id == UnitId::ZergCagrantColony || unit_id == UnitId::ProtossWarden) {
							u32 region_id = helper::get_region_id(build_pos.x, build_pos.y);
							helper::do_area_fixups_for_enemy(player, region_id, 4);
						}
						if (g_aiscript_controller[player].failed_placement) {
							return helper::ai_finish_unit(unit);
						}
						building_placement = helper::ai_manage_building_placement(unit, unit_id, build_pos, &build_pos, 64);
					}
					if (!building_placement) {
						g_aiscript_controller[player].failed_placement = true;
						return helper::ai_finish_unit(unit);
					}
				}
			}
		}

		helper::ai_reserve(player, 1);
		helper::ai_pop_spending_request(player);

		u32 order_id = ai_get_build_order_id(unit, unit_id);
		if (order_id != OrderId::None) {
			helper::issue_order_targeting_ground(unit, order_id, build_pos.x, build_pos.y);
			helper::refund_all_queue_slots(unit);
			helper::has_money_can_make(unit, unit_id);
			return true;
		}
		return false;
	}

	// Hooked to improve request ordering. -Veeq7
	bool ai_add_spending_request(u32 id, u32 player, u32 type, u32 priority, void* value) { // 00447980
		switch (type) {
			case RequestType::Military:
			case RequestType::Guard:
			case RequestType::Worker:
			case RequestType::Witness:
			{
				if (helper::ai_isnt_at_define_max(player, id)) {
					return false;
				}
			}
		}

		AI_Main* player_ai = &g_aiscript_controller[player];
		if (player_ai->request_count < 63) {
			player_ai->request_count += 1;
		}

		u32 index = player_ai->request_count - 1;
		player_ai->requests[index].id = id;
		player_ai->requests[index].type = type;
		player_ai->requests[index].priority = priority;
		player_ai->requests[index].value = value;

		while (index > 0) {
			u32 parent_index = (index - 1) / 2;
			SpendingRequest request        = player_ai->requests[index];
			SpendingRequest parent_request = player_ai->requests[parent_index];
			if (request.priority <= parent_request.priority) {
				break;
			}
			player_ai->requests[index] = parent_request;
			player_ai->requests[parent_index] = request;
			index = parent_index;
		}
		return true;
	}

	// Hooked in case it's useful for improving AI detector requests. -Pr0nogo
	s32 ai_make_detector_behavior(Unit* unit, u32 player_id) { // 00435130
		if (g_aiscript_controller[player_id].requests[0].id == UnitId::ProtossWitness) {
			if (helper::is_unit_id_race_equal_to_unit_race(UnitId::ProtossWitness, unit)
			&&  helper::create_unit_allowed(UnitId::ProtossWitness, unit, player_id) == 1) {
				if (helper::has_resources_for_unit(UnitId::ProtossWitness, player_id)) {
					if (helper::ai_isnt_at_define_max(player_id, UnitId::ProtossWitness)) {
						helper::ai_reserve(player_id, 0);
						helper::ai_pop_spending_request(player_id);
						return 0;
					}
					if (helper::ai_train_unit(unit, UnitId::ProtossWitness, 5, 0)) {
						helper::set_secondary_order(unit, OrderId::Train);
						helper::ai_reserve(player_id, 1);
						helper::ai_pop_spending_request(player_id);
						return 1;
					}
				}
			}
			else if (*g_last_internal_error == 2
			||		 *g_last_internal_error == 23
			||		 *g_last_internal_error == 21
			||		 *g_last_internal_error == 8
			||		 *g_last_internal_error == 7
			||		 *g_last_internal_error == 13) {
				helper::ai_reserve(player_id, 0);
				helper::ai_pop_spending_request(player_id);
				helper::ai_perform_task(unit);
			}
		}
		return 0;
	}

	// Hooked to investigate refinery issue. -Pr0nogo
	bool has_resources_for_unit(u16 unit_id, u32 player) { // 004487B0
		g_ai_minerals_reserve[player] = units_dat::mineral_cost[unit_id];
		g_ai_vespene_reserve[player] = units_dat::vespene_cost[unit_id];
		g_ai_supply_reserved[player] = 0;
		// Originally also checked supplies.
		if (g_game_minerals[player] < g_ai_minerals_reserve[player]) {
			helper::display_error_message_id_with_sfx(player, 850, *g_console_index + 147);
			return false;
		}
		if (g_game_vespene[player] < g_ai_vespene_reserve[player]) {
			helper::display_error_message_id_with_sfx(player, 851, *g_console_index + 150);
			return false;
		}
		if (g_aiscript_controller[player].minerals_available < g_ai_minerals_reserve[player]
			|| g_aiscript_controller[player].vespene_available < g_ai_vespene_reserve[player]) {
			return false;
		}
		return true;
	}

	// Hooked to change position check from sprite to unit. -Pr0nogo
	void ai_make_guard(u32 player, Unit* unit) { // 0043A010
		u32 region_id = scbw::get_region_id(unit->position);
		AiRegion* ai_region = scbw::get_ai_region(player, region_id);
		char region_state = ai_region->state;
		if (!region_state || region_state == RegionState::TownRegionLow) {
			helper::ai_set_region_state(RegionState::TownRegionHigh, ai_region);
			ai_region->flags |= 0x1;
			if (!g_aiscript_controller[player].ai_flags.is_use_map_settings) {
				ai_region->flags |= 0x40;
			}
			AiRegion* ai_region = scbw::get_ai_region(player, scbw::get_region_id(unit->position));
			for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
				if (!g_aiscript_controller[other_player].ai_flags.is_use_map_settings) {
					if (scbw::is_allied_to(other_player, player)) {
						AiRegion* other_player_ai_region = scbw::get_ai_region(other_player, region_id);
						if (!other_player_ai_region->state) {
							helper::ai_set_region_state(RegionState::TownRegionLow, other_player_ai_region);
						}
					}
				}
			}
		}
		if ((unit->status & UnitStatus::Completed
		||	 scbw::unit_id_is_zerg_production(unit->id, true)
		||   unit->id == UnitId::ZergHachirosk && helper::unit_is_morphing(unit))
		&&   unit->status & UnitStatus::GroundedBuilding && (units_dat::base_property[unit->id] & UnitProperty::ResourceDepot)
		||	 scbw::unit_id_is_town_center(unit->id)) { // Originally checked for race, so Terran would only accept Command Center, etc
			if (g_aiscript_controller[player].ai_flags.is_use_map_settings) {
				ai_region->ground_strength_needed = 6 * g_aiscript_controller[player].default_min;
				ai_region->air_strength_needed = 6 * g_aiscript_controller[player].default_min;
				helper::ai_set_region_state(RegionState::Defending, ai_region);
			} else {
				ai_region->ground_strength_needed = 1500;
				ai_region->air_strength_needed = 1000;
				helper::ai_set_region_state(RegionState::Defending, ai_region);
				ai_region->flags |= 0x40;
			}
			ai_region->defend_time = 45;
		}
	}

	// Hooked to lay the groundwork for more refinery types. -Pr0nogo
	u32 get_default_refinery_type(u32 player) // 004320D0
	{
		switch (g_player_table[player].race)
		{
			case RaceId::Zerg:
			{
				return UnitId::ZergExcisant;
			}
			case RaceId::Terran:
			{
				return UnitId::TerranReservoir;
			}
			case RaceId::Protoss:
			{
				return UnitId::ProtossAquifer;
			}
		}
	}
}