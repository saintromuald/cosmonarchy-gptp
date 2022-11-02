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
		AiTown* result = gi_ai_active_town_array[player].first;
		AiTown* first_town = gi_ai_active_town_array[player].first;
		if (result)
		{
			do
			{
				WorkerAi* first_worker = first_town->workers;
				if (first_town->workers)
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
										console_var = console_index;
										helper::display_error_message_id_with_sfx((u8)player, 851, *console_var + 150);
									}
								}
								else
								{
									console_var = console_index;
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
	
	enum RequestType
	{
		RequestType_UnitMake = 0,
		RequestType_UnitMergeMorph1 = 1,
		RequestType_UnitMergeMorph2 = 2,
		RequestType_ConstructMorph = 3,
		RequestType_Train = 4,
		RequestType_Upgrade = 5,
		RequestType_Research = 6,
		RequestType_PlaceAddon = 7,
		RequestType_MakeDetector = 8,
	};

	char ai_perform_task(Unit* unit) // 004361A0 -- hooked to fix AI Archon training -- Pr0nogo
	{
		switch (unit->id)
		{
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

		UnitAi* unit_ai_var = (UnitAi*)unit->unit_ai;
		if (!unit_ai_var || unit_ai_var->type != AiType::Building)
		{
			return 1;
		}

		s32 player = unit->player;
		s32 some_bool = 0;
		if (g_game_minerals[player] > 1500 && g_game_vespene[player] > 1500)
		{
			if (!helper::unit_is_training_or_morphing(unit))
			{
				if (helper::ai_unit_construction_behavior(player, unit, 160))
				{
					some_bool = 1;
				}
			}
		}

		AI_Main* player_ai_data = &g_aiscript_controller[player];
		s32 request_type;
		if (player_ai_data->request_count)
		{
			request_type = player_ai_data->requests->type;
			if (player_ai_data->requests->id == UnitId::ProtossArchon || player_ai_data->requests->id == UnitId::ProtossMindTyrant)
			{
				// Override Archon and Mind Tyrant request types
				request_type = RequestType_Train;
			}
		}
		else
		{
			request_type = 0;
		}

		switch (request_type)
		{
			case RequestType_UnitMake:
			{
				if (!helper::ai_unit_construction_behavior(player, unit, 50))
				{
					return 1;
				}
				some_bool = 1;
				return helper::ai_plan_do_merge_morph(player, unit, some_bool);
			}
			case RequestType_UnitMergeMorph1:
			case RequestType_UnitMergeMorph2:
			{
				return helper::ai_plan_do_merge_morph(player, unit, some_bool);
			}
			case RequestType_ConstructMorph:
			{
				return helper::ai_morph_behavior(player, unit, unit->unit_ai);
			}
			case RequestType_Train:
			{
				helper::ai_train_behavior(unit, unit->unit_ai, player);
				return 1;
			} break;
			case RequestType_Upgrade:
			{
				helper::ai_upgrade_behavior(unit, player);
				return 1;
			} break;
			case RequestType_Research:
			{
				helper::ai_research_behavior(unit, player);
				return 1;
			} break;
			case RequestType_PlaceAddon:
			{
				helper::ai_construct_behavior(unit, unit->unit_ai, player);
				return 1;
			} break;
			case RequestType_MakeDetector:
			{
				return helper::ai_make_detectors_behavior(unit, player);
			} break;
		}
		return 1;
	}

	char ai_plan_do_merge_morph(s32 player, Unit* unit, s32 boolSomething) // 00435F10
	{
		bool error;
		AiTown* mainTown2;
		char requestedBuildId;
		AI_Main* mainTown;
		s16 mainTownTown;
		s16 unitAIQueues;
		s16 unit_id;

		Unit* unit_copy = unit;
		s32 nextBuildType = g_aiscript_controller[player].requests->type;
		Unit* dummy = nullptr;
		switch (nextBuildType)
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

		if (!helper::is_unit_id_race_equal_to_unit_race(nextBuildType, unit_copy) || helper::create_unit_allowed(nextBuildType, unit_copy, player) != 1)
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
			error = *g_last_internal_error == 13;
			if (error)
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

		if (!helper::hasResourcesForUnit(nextBuildType, player))
		{
			error = (g_aiscript_controller[player].ai_flags.is_use_map_settings) == 0;
			if (error)
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

		if (helper::is_ai_allowed_to_build(player, nextBuildType))
		{
			helper::ai_reserve(player, 0);
			helper::ai_pop_spending_request(player);
			return 0;
		}

		if (unit == unit_copy)
		{
			requestedBuildId = helper::ai_get_request_build_type(player);
			if (!helper::ai_train_unit(unit_copy, nextBuildType, requestedBuildId, g_aiscript_controller[player].requests->val))
			{
				return 0;
			}
		}
		else
		{
			if (!helper::has_money_can_make(unit_copy, nextBuildType))
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
				helper::add_military_ai((AiRegion*)g_aiscript_controller[player].requests->val, unit_copy, 1);
			}
		}

		helper::ai_reserve(player, 1);
		helper::ai_pop_spending_request(player);
		unit_id = unit_copy->id;
		if (scbw::is_conventional_unit_morph_source(unit_id))
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
						g_aiscript_controller[player_id].mineral_needed += units_dat::mineral_cost[UnitId::ProtossWitness];
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
						g_aiscript_controller[player_id].mineral_needed += units_dat::mineral_cost[transport_unit_id_1];
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

		if (!unit->unit_ai || unit->unit_ai->type != AiType::Military) {
			return false;
		}

		// Skip current attack force units
		AiRegion* current_ai_region = unit->unit_ai->military_ai.region;
		if (scbw::is_attack_region(current_ai_region)) {
			return false;
		}

		if (strict) {
			// Skip transports
			if (scbw::unit_id_is_transport(unit->id)) {
				return false;
			}

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

	// Hooked to improve AI guard handling. -Pr0nogo
	void ai_update_guard_needs(u32 player) { // 004630C0
		GuardAi* prev = 0;
		for (GuardAi* guard_ai = g_first_player_guard->player[player].first; guard_ai; guard_ai = prev) {
			prev = guard_ai->link.prev;
			if (guard_ai->times_died >= 100) {
				helper::delete_guard_ai(guard_ai, &g_first_player_guard->player[player]);
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
				Unit* nearest_military = helper::find_nearest_available_military(player, guard_ai->unit_id, guard_ai->original_home);
				if (nearest_military && scbw::get_distance_fast(guard_ai->original_home, nearest_military->position) < 30 * 32) {
					helper::remove_unit_ai(nearest_military, 0);
					guard_ai->current_home = guard_ai->original_home;
					guard_ai->parent = nearest_military;
					nearest_military->unit_ai = (UnitAi*)guard_ai;
					helper::issue_order_targeting_nothing(nearest_military, OrderId::ComputerAI);
				}
				// Request handling is currently in Aise, so we don't duplicate the priority system
				//else if (!guard_ai->previous_update || *g_elapsed_time_seconds > guard_ai->previous_update + 5) {
				//	helper::ai_request_guard(guard_ai, player);
				//	guard_ai->previous_update = *g_elapsed_time_seconds;
				//}
			}
		}
	}

	// Hooked to add sprite nullcheck and improve behavior. -Pr0nogo
	void ai_make_guard(u32 player_id, Unit* unit) { // 0043A010
		if (!unit || !unit->sprite) {
			return;
		}
		u16 region_id = helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y);
		u32 region_id_var = region_id;
		AiRegion* region_from_id = &g_ai_region_captains[player_id][region_id_var];
		u32 region_times_52 = region_id_var * 52;

		if (!region_from_id->state || region_from_id->state == RegionState::TownRegionLow) {
			helper::ai_set_region_type(5, region_from_id);
			u8 region_flags = region_from_id->flags | 1;
			region_from_id->flags = region_flags;
			if (!g_aiscript_controller[player_id].ai_flags.is_use_map_settings) {
				region_from_id->flags = region_flags | 0x40;
			}

		//	u32 ai_flags = g_aiscript_controller[0].ai_flags; // Really weird
			u8* alliance_check = (u8*)player_alliance + player_id;
			AiRegion** ai_regions_var = g_ai_region_captains;
			u8(*player_alliance_var)[12] = (u8(*)[12])((u8*)player_alliance + player_id);
			do {
				if (!g_aiscript_controller[0].ai_flags.is_use_map_settings) {
					if (*alliance_check) {
						AiRegion* region_from_52 = (AiRegion*)((char*)*ai_regions_var + region_times_52);
						if (!region_from_52->state)
						{
							helper::ai_set_region_type(4, region_from_52);
						}
					}
				}
			//	ai_flags += 628;
				alliance_check = &(*player_alliance_var)[12];
				++player_alliance_var;
				++ai_regions_var;
			} while (1); //ai_flags < &g_base_array[9].total_mineral_count;
		}

		u32 unit_status = unit->status;
		u32 town_center = UnitId::TerranCommandCenter;
		if (g_player_table[player_id].race == RaceId::Protoss) {
			town_center = UnitId::ProtossNexus;
		} else if (g_player_table[player_id].race == RaceId::Zerg) {
			town_center = UnitId::ZergHachirosk;
		}

		u32 unit_id = unit->id;
		u8 player_race = g_player_table[player_id].race;
		if ((unit_status & UnitStatus::Completed
		||   unit_id == UnitId::ZergSibraelosk // new
		||   unit_id == UnitId::ZergGathtelosk
		||   unit_id == UnitId::ZergCaphrolosk
		||   unit_id == UnitId::ZergHachirosk && helper::unit_is_morphing(unit))
		&&   unit_status & UnitStatus::GroundedBuilding && (units_dat::base_property[unit->id] & UnitProperty::Subunit) // was BYTE1, never saw that on a flag var before, so maybe Subunit should be FlyingBuilding (next entry)?
		||  (unit->id == town_center)) {
			if (g_aiscript_controller[player_id].ai_flags.is_use_map_settings) {
				region_from_id->ground_strength_needed = 6 * g_aiscript_controller[player_id].default_min;
				region_from_id->air_strength_needed = 6 * g_aiscript_controller[player_id].default_min;
				helper::ai_set_region_type(6, region_from_id);
			} else {
				helper::ai_set_region_type(6, region_from_id);
				u8 region_has_flag_40 = region_from_id->flags | 0x40;
				region_from_id->ground_strength_needed = 1500;
				region_from_id->air_strength_needed = 1000;
				region_from_id->flags = region_has_flag_40;
			}
			region_from_id->defend_time = 45;
		}
	}

	// Hooked to fix player_need and improve functionality. -Pr0nogo
	void ai_add_spending_request_from_towns(u32 player) { // 00436320
		g_aiscript_controller[player].ai_flags.upgrades_finished = true; // HIBYTE(g_aiscript_controller[player].ai_flags) |= 4u;

		u32 race = g_player_table[player].race;
		u32 builder_id = UnitId::None;
		u32 main_building_id = UnitId::None;
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
		for (town = gi_ai_active_town_array[player].first; town; town = town->prev) {
			helper::cancel_queued_trains(town);
			if (town->workers) {
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
					u32 priority_subtract = 0;
					if (request->is_upgrade || request->is_tech) {
						u32 request_type = 5;
						u32 mineral_cost = upgrades_dat::mineral_cost_base[request->id] + upgrades_dat::mineral_cost_factor[request->id] * scbw::get_upgrade_level(player, request->id);
						u32 vespene_cost = upgrades_dat::mineral_cost_base[request->id] + upgrades_dat::vespene_cost_factor[request->id] * scbw::get_upgrade_level(player, request->id);
						if (request->is_tech) {
							request_type = 6;
							mineral_cost = techdata_dat::mineral_cost[request->id];
							vespene_cost = techdata_dat::vespene_cost[request->id];
						}
						g_aiscript_controller[player].mineral_needed += mineral_cost * count;
						g_aiscript_controller[player].vespene_needed += vespene_cost * count;
						for (u32 i = 0; i < count; ++i) {
							helper::ai_add_spending_request(request->id, player, request_type, request->priority, town);
						}
					} else {
						scbw::town_add_spending_request(town, count, request->id, request->priority);
					}
				}
			}

			if (town->worker_count > 1 || (town->worker_count == 1 && town->main_building && race == RaceId::Protoss)) {
				hooks::ai_build_refinery(town);
			}
		}

		// Last town handling
		if (town) {
			if (has_excess_workers) {
				for (AiTown* town = gi_ai_active_town_array[player].first; town; town = town->prev) {
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
}