#include "unit_ai.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"
#include "globals.h"

namespace hooks
{
	void ai_training_unit(Unit* result, Unit* parent) { // 004A2830
		if (playerTable[result->player].type != PlayerType::Computer) {
			return;
		}

		if (result->id == UnitId::TerranCleric && !AIScriptController[result->player].main_medic) {
			AIScriptController[result->player].main_medic = result;
		}

		if (units_dat::base_property[result->id] & UnitProperty::Worker) {
			helper::ai_training_worker(result, parent);
		} else if (scbw::unit_id_is_zerg_supply(result->id)) {
			helper::ai_training_overlord(result, parent); // replace with hook later
		} else if (result->id == UnitId::ZergRilirokor
				|| result->id == UnitId::ZergRililisk
				|| result->id == UnitId::ProtossArchon
				|| result->id == UnitId::ProtossMindTyrant) {
			// archon and mind tyrant should be moved to ai_trainingnormal, once it's hooked and archon's hardcode is removed
			hooks::ai_training_simple_military(result, parent);
		} else {
			helper::ai_training_normal(result, parent);
		}
	}

	// Hooked to add region null check - Pr0nogo
	void ai_training_simple_military(Unit* result, Unit* parent) // 0043E280
	{
		if (units_dat::base_property[result->id] & (UnitProperty::Worker | UnitProperty::Building))
		{
			scbw::printf("SimpleMilitary on worker or building! ID: %i", result->id);
		}

		s32 pX, pY;
		if (result->sprite != nullptr)
		{
			pX = result->sprite->position.x;
			pY = result->sprite->position.y;
		}
		else
		{
			pX = result->position.x;
			pY = result->position.y;
			GPTP::logger << "null sprite of Create Broodling detected!" << '\n';
		}

		s32 regionId = helper::get_region_id(pX, pY);
		AiRegion* playerRegion = AiRegionCaptains[parent->player];
		if (playerRegion == nullptr)
		{
			GPTP::logger << "null region in AI_TrainingBroodling!" << '\n';
			return;
		}

		AiRegion* region = &playerRegion[regionId];
		if (region == nullptr)
		{
			GPTP::logger << "AI Broodling Bug: Null region!" << '\n';
		}
		else
		{
			if (playerRegion[regionId].state != RegionState::Nothing)
			{
				helper::ai_set_region_type(1, region);
			}
			helper::add_military_ai(region, result, 1);
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	void ai_training_worker(Unit* unit, Unit* parent) // 00435700
	{
		AiTown* town = 0;
		WorkerAi* parent_ai = (WorkerAi*)parent->unit_ai;
		if (parent_ai && (parent_ai->type == AiType::Worker || parent_ai->type == AiType::Building)) {
			town = parent_ai->town;
		} else {
			town = scbw::get_closest_town(unit->player, unit->position);
		}

		if (parent == unit) {
			helper::ai_remove_from_towns(unit);
		}

		if (unit->unit_ai && unit->unit_ai->type != AiType::Building) {
			helper::remove_unit_ai(unit, unit->unit_ai); // Sanity removal for cases when workers have military AI for some reason
		}

		if (parent == unit)
		{
			WorkerAi* unit_ai = (WorkerAi*)unit->unit_ai;
			if (unit_ai)
			{
				AiTown* town = unit_ai->town;
				if (town && unit_ai->type == AiType::Building) // type check is new
				{
					helper::delete_building_ai((BuildingAi*)unit_ai, &town->free_buildings); // moved within nullcheck for sanity
					if (town->building_SCV == unit)
					{
						town->building_SCV = 0;
					}
					if (town->main_building == unit)
					{
						town->main_building = 0;
					}
				}
				unit->unit_ai = 0;
			}
		}

		if (town) {
			helper::add_worker_structure_ai(unit, town);
		}
	}

	// Hooked to investigate crashes and remove Archon hardcode - Pr0nogo
	void ai_training_normal(Unit* newUnit, Unit* parentUnit) // 00435DB0
	{
		s16 unit_id = newUnit->id;
		if (!scbw::is_conventional_unit_morph_result(unit_id))
		{
			if (newUnit->status & UnitStatus::IsHallucination)
			{
				helper::add_military_ai(&AiRegionCaptains[(u8)newUnit->player][helper::get_region_id(newUnit->sprite->position.x, newUnit->sprite->position.y)], newUnit, 1);
				return;
			}
			if (unit_id != UnitId::TerranNuclearMissile && unit_id != UnitId::ProtossWitness)
			{
				BuildingAi* parentUnitAi = (BuildingAi*)parentUnit->unit_ai;
				BuildingAi* parentUnitAiBackup = (BuildingAi*)parentUnit->unit_ai;
				if (parentUnitAi)
				{
					s32 queueSlot = 0;
					if (parentUnit == newUnit)
					{
						queueSlot = 0;
						helper::ai_remove_from_towns(newUnit);
						BuildingAi* newUnitAi = (BuildingAi*)newUnit->unit_ai;
						if (newUnitAi)
						{
							AiTown* newUnitTown = newUnitAi->town;
							helper::delete_building_ai(newUnitAi, &newUnitTown->free_buildings);
							if (newUnitTown)
							{
								if (newUnit == newUnitTown->building_SCV)
								{
									newUnitTown->building_SCV = 0;
								}
								if (newUnit == newUnitTown->main_building)
								{
									newUnitTown->main_building = 0;
								}
							}
							parentUnitAi = parentUnitAiBackup;
							newUnit->unit_ai = 0;
						}
					}
					else if (parentUnit->id == unit_id)
					{
						queueSlot = 0;
					}
					else
					{
						queueSlot = (u8)parentUnit->build_queue_slot;
					}
					AiRegion* region = (AiRegion*)parentUnitAi->trainQueueValues[queueSlot];
					if (parentUnitAi->trainQueueTypes[queueSlot] == 2)
					{
						if (!region->ground_strength_needed)
						{
							s16 regionFullAir = region->air_strength_full;
							s16 regionEnemyAir = region->air_strength_enemy;
							region->ground_strength_needed = (u16)newUnit;
							region->air_strength = regionFullAir;
							region->ground_strength_full = regionEnemyAir;
							newUnit->unit_ai = (UnitAi*)region;
							return;
						}
						parentUnitAi->trainQueueTypes[queueSlot] = 1;
						region = helper::getAiRegionInfoFromUnitLoc(newUnit);
						parentUnitAi->trainQueueValues[queueSlot] = (void*)region;
					}
					if (parentUnitAi->trainQueueTypes[queueSlot] == 1)
					{
						helper::add_military_ai(region, newUnit, 0);
					}
					else
					{
						helper::add_guard_ai_to_unit(newUnit);
					}
				}
			}
		}
	}

	// Hooked because iquare told me to - Pr0nogo
	void* add_worker_structure_ai(Unit* unit, AiTown* town) // 00433DD0
	{
		void* result = 0;
		if (units_dat::base_property[unit->id] & UnitProperty::Worker)
		{
			if (town->free_workers) {
				WorkerAi* ai = 0;
				if (town && town->free_workers) {
					if (town->free_workers <= (void*)0x100000) {
						scbw::printf("Corrupted free worker AI array!");
						return result;
					}
					if (town->free_workers->first_free_worker <= (void*)0x100000) {
						scbw::printf("Corrupted first free worker AI!");
						return result;
					}
					ai = town->free_workers->first_free_worker; // This nullcheck was added
				}
				if (ai)
				{
					result = ai;
					WorkerAi* worker_prev = ai->link.prev;
					town->free_workers->first_free_worker = ai->link.prev;
					if (worker_prev) {
						worker_prev->link.next = 0;
					}
					ai->link.next = 0;
					ai->link.prev = town->workers;
					if (town->workers)
					{
						town->workers->link.next = ai;
					}
					town->workers = ai;
					ai->type = 2;
					ai->parent = unit;
					ai->town = town;
					ai->target_resource = 1;
					ai->last_update_second = *elapsed_time_seconds;
					ai->wait_timer = 0;
					ai->reassign_count = 0;
					unit->worker.target_resource.unit = 0;
					unit->unit_ai = (UnitAi*)ai;
					++town->worker_count;

					if (!town->inited)
					{
						town->inited = 1;
						town->mineral = 0;
						town->gas_buildings[0] = 0;
						town->gas_buildings[1] = 0;
						town->gas_buildings[2] = 0;

						UnitExt* unitExt = globals.get_unit_ext(unit);
						unitExt->values[ValueAI_WorkerChosenForExpansion] = 1;
						unitExt->values[ValueAI_WorkerChosenX] = town->position.x;
						unitExt->values[ValueAI_WorkerChosenY] = town->position.y;
					}
				}
			}
		}
		else if ((units_dat::base_property[unit->id] & UnitProperty::Building && unit->id != UnitId::ResourceVespeneGeyser) ||
			unit->id == UnitId::ZergLarva || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergOvileth)
		{
			BuildingAi* ai = 0;
			if (town && town->free_buildings) {
				if (town->free_buildings <= (void*)0x100000) {
					scbw::printf("Corrupted free building AI array!");
					return result;
				}
				if (town->free_buildings->first_free_building <= (void*)0x100000) {
					scbw::printf("Corrupted first free building AI!");
					return result;
				}
				ai = helper::create_building_ai(&town->free_buildings); // This nullcheck was added
			}
			if (ai) {
				result = ai;
				ai->type = 3;
				ai->parent = unit;
				ai->town = town;
				ai->trainQueueValues[0] = 0;
				ai->trainQueueValues[1] = 0;
				ai->trainQueueValues[2] = 0;
				ai->trainQueueValues[3] = 0;
				ai->trainQueueValues[4] = 0;
				ai->trainQueueTypes[0] = 0;
				ai->trainQueueTypes[1] = 0;
				ai->trainQueueTypes[2] = 0;
				ai->trainQueueTypes[3] = 0;
				ai->trainQueueTypes[4] = 0;
				unit->unit_ai = (UnitAi*)ai;
				if (unit->id == UnitId::ZergHatchery || unit->id == UnitId::ZergLair || unit->id == UnitId::ZergHive || unit->id == UnitId::ZergSire)
				{
					if (unit->status & UnitStatus::Completed)
					{
						unit->order_computer(OrderId::ComputerAI);
						helper::set_secondary_order(unit, OrderId::SpreadCreep);
					}
				}
				if (!(AIScriptController[town->player].AI_Flags.is_use_map_settings)
					|| unit->status & UnitStatus::GroundedBuilding
					&& unit->id != UnitId::TerranWatchdog
					&& unit->id != UnitId::TerranBunker
					&& unit->id != UnitId::ZergCreepColony
					&& unit->id != UnitId::ZergSporeColony
					&& unit->id != UnitId::ZergSunkenColony
					&& unit->id != UnitId::ProtossPylon
					&& unit->id != UnitId::ProtossPhotonCannon)
				{
					helper::ai_make_guard(town->player, unit);
				}
				if (!town->inited)
				{
					town->inited = 1;
					town->mineral = 0;
					town->gas_buildings[0] = 0;
					town->gas_buildings[1] = 0;
					town->gas_buildings[2] = 0;
				}

				u32 town_hall_id = UnitId::ZergHatchery;
				if (playerTable[unit->player].race == RaceId::Terran)
				{
					town_hall_id = UnitId::TerranCommandCenter;
				}
				else if (playerTable[unit->player].race == RaceId::Protoss)
				{
					town_hall_id = UnitId::ProtossNexus;
				}

				if (unit->id == town_hall_id || helper::unit_is_active_resource_depot(unit))
				{
					if (!town->main_building)
					{
						town->main_building = unit;
					}
				}
				else
				{
					u32 refinery_id = helper::get_default_refinery_type(unit->player);
					if (unit->id == refinery_id || unit->id == UnitId::ResourceVespeneGeyser)
					{
						if (!town->gas_buildings[0])
						{
							town->gas_buildings[0] = unit;
						}
						else if (!town->gas_buildings[1])
						{
							town->gas_buildings[1] = unit;
						}
						else if (!town->gas_buildings[2])
						{
							town->gas_buildings[2] = unit;
						}
						if (town->resarea)
						{
							town->resource_units_not_set = 1;
						}
					}
				}
			}
		}
		return result;
	}

	// Hooked to rule out AI crashes - Pr0nogo
	void ai_create_nuke(Unit* silo, Unit* result)	// 0045B7A0
	{												// originally had 3 args; last one was unused
		if (silo->id == UnitId::TerranMissileSilo && !silo->building.silo.is_ready)
		{
			Unit* nuke = helper::create_unit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (u8)silo->player);
			if (nuke)
			{
				helper::update_unit_stats_and_finish_building(nuke);
				helper::update_unit_strength_and_apply_default_orders(nuke);
				helper::hide_unit(nuke);
				silo->building.silo.nuke = nuke;
				silo->building.silo.is_ready = 1;
				nuke->connected_unit = silo;
				ai_training_unit(nuke, silo);
			}
		}
	}


	// Hooked to investigate improperly-initialized unit AI, and to add functionality. -Pr0nogo
	void ai_add_military_to_region(AiRegion* region, Unit* unit, u32 must_use_region) // 0043DA20
	{
		AiRegion* final_region = region;
		if (region->state == RegionState::EnemySighted && !must_use_region)
		{
			final_region = &AiRegionCaptains[unit->player][helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y)];
		}
		MilitaryAiArray* military_ai_array = final_region->military;
		MilitaryAi* first_free_military = military_ai_array->first_free_military;
		if (first_free_military)
		{
			MilitaryAi* prev_military = first_free_military->link.prev;
			military_ai_array->first_free_military = (MilitaryAi*)first_free_military->link.prev;
			if (first_free_military->link.prev)
			{
				prev_military->link.next = 0;
			}
			if (first_free_military)
			{
				first_free_military->link.next = 0;
				first_free_military->link.prev = final_region->first_military_ai;
				MilitaryAi* first_military = final_region->first_military_ai;
				if (first_military)
				{
					first_military->link.next = first_free_military;
				}
				final_region->first_military_ai = first_free_military;
				first_free_military->type = AiType::Military;
				first_free_military->unit = unit;
				first_free_military->region = final_region;
				if (!(unit->status & UnitStatus::InAir))
				{
					++final_region->ground_unit_count;
				}
				unit->unit_ai = (UnitAi*)first_free_military;
				helper::ai_order_to_destination(unit, unit->id != UnitId::TerranCleric ? 157 : 177, (*SAI_Paths)->regions[final_region->region].region_center_x >> 8, (*SAI_Paths)->regions[final_region->region].region_center_y >> 8);
				char region_state = final_region->state;
				if (region_state == RegionState::PreparingAttack || region_state == RegionState::PreparingAttackOldUnreachable || region_state == RegionState::ContinueAttackHere || region_state == RegionState::Attacking)
				{
					helper::ai_assign_region_to_slowest_unit(final_region);
				}
			}
		}
	}

	// Hooked to change guard assignments for structures based on AI personality -- Pr0nogo
	// TODO: Cleanup and make changes
	void iterate_building_ai(Unit* result, Unit* parent) // 00435770
	{
		UnitAi* parentAI;
		AiTown* parentTown;
		BuildingAi* removeBuilding;
		AiTown* removeBuildingTown;
		Unit* workerUnit;
		Unit* workerTarget;
		s32 queueCount;
		s32 queueCount2;
		BuildingAi* j;
		Unit* builderUnit;
		u32 i;
		WorkerAi* worker;
		BuildingAi* building;

		if (playerTable[(u8)parent->player].type == PlayerType::Computer)
		{
			parentAI = (UnitAi*)parent->unit_ai;
			if (parentAI)
			{
				if (parentAI->type == 2)
				{
					worker = (WorkerAi*)parentAI;
					parentTown = worker->town;
				}
				else
				{ // assumed BuildingAi, unlikely to be GuardAi, can't be anything else
					building = (BuildingAi*)parentAI;
					parentTown = building->town;
				}
				if (parent == result)
				{
					helper::ai_remove_from_towns(result);
					removeBuilding = (BuildingAi*)result->unit_ai;
					if (removeBuilding)
					{
						removeBuildingTown = removeBuilding->town;
						helper::delete_building_ai(removeBuilding, &removeBuildingTown->free_buildings);
						if (removeBuildingTown)
						{
							if (result == removeBuildingTown->building_SCV)
							{
								removeBuildingTown->building_SCV = 0;
							}
							if (result == removeBuildingTown->main_building)
							{
								removeBuildingTown->main_building = 0;
							}
						}
						result->unit_ai = 0;
					}
				}
				helper::add_worker_structure_ai(result, parentTown);
				if (result->id != UnitId::TerranBunker || *elapsed_time_seconds <= 60)
				{
					if (result->id == UnitId::TerranWatchdog)
					{
						workerUnit = (Unit*)parentTown->workers;
						for (i = 0; workerUnit; i += queueCount)
						{
							workerTarget = workerUnit->move_target.unit;
							if (workerTarget->id == UnitId::TerranWatchdog)
							{
								queueCount = 1;
							}
							else
							{
								queueCount = helper::ai_get_queued_count(workerTarget, UnitId::TerranWatchdog);
							}
//							workerUnit = *(u32*)workerUnit; // not sure if this is even necessary, does v8 need to be iterated upon every loop?
						}
						for (j = parentTown->buildings; j; i += queueCount2)
						{
							builderUnit = j->parent;
							if (builderUnit->id == UnitId::TerranWatchdog)
							{
								queueCount2 = 1;
							}
							else
							{
								queueCount2 = helper::ai_get_queued_count(builderUnit, UnitId::TerranWatchdog);
							}
							j = j->link.prev;
						}
						if (i >= 4)
						{
							helper::prepare_guard_ai(result->sprite->position.y, (u8)result->player, 1, result->sprite->position.x);
						}
					}
				}
				else
				{
					helper::prepare_guard_ai(result->sprite->position.y, (u8)result->player, 5, result->sprite->position.x);
					helper::prepare_guard_ai(result->sprite->position.y, (u8)result->player, 1, result->sprite->position.x);
				}
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	void remove_unit_ai(Unit* unit, UnitAi* unitAI) // 004A1E50
	{
		helper::remove_from_ai_structs(unit, unitAI);
		helper::remove_worker_or_building_ai(unit, nullptr);
		if (unitAI != nullptr)
		{
			if (unitAI->type == AiType::Guard)
			{
				GuardAi* guardAI = (GuardAi*)unitAI;
				u8 timesDied = guardAI->times_died;
				guardAI->parent = nullptr;
				if ((u8)timesDied < 100u)
				{
					guardAI->times_died = timesDied + 1;
				}
			}
		}
		helper::ai_remove_medic(unit);
		unit->unit_ai = nullptr;
	}

	// Hooked to investigate crashes - Pr0nogo
	void delete_worker_ai(WorkerAi* workerAi, WorkerAiArray* workerAiArray) // 00404470
	{
		// New
		if (workerAi == nullptr)
		{
			return;
		}

		if (workerAiArray->worker_ais[0].link.next == workerAi)
		{
			workerAiArray->worker_ais[0].link.next = workerAi->link.prev;
		}

		if (workerAi->link.next)
		{
			workerAi->link.next->link.prev = workerAi->link.prev;
		}

		if (workerAi->link.prev)
		{
			workerAi->link.prev->link.next = workerAi->link.next;
		}

		workerAi->link.next = nullptr;
		workerAi->link.prev = workerAiArray->first_free_worker;
		if (workerAiArray->first_free_worker)
		{
			workerAiArray->first_free_worker->link.next = workerAi;
		}
		workerAiArray->first_free_worker = workerAi;
	}

	// Hooked to resolve a crash - Pr0nogo
	void delete_building_ai(BuildingAi* buildingAi, BuildingAiArray* buildingAiArray) // 00404500
	{
		// New
		if (buildingAi == nullptr)
		{
			return;
		}

		if (buildingAiArray->building_ais[0].link.next == buildingAi)
		{
			buildingAiArray->building_ais[0].link.next = buildingAi->link.prev;
		}

		if (buildingAi->link.next)
		{
			buildingAi->link.next->link.prev = buildingAi->link.prev;
		}
		if (buildingAi->link.prev)
		{
			buildingAi->link.prev->link.next = buildingAi->link.next;
		}

		buildingAi->link.next = nullptr;
		buildingAi->link.prev = buildingAiArray->first_free_building;
		if (buildingAiArray->first_free_building)
		{
			buildingAiArray->first_free_building->link.next = buildingAi;
		}
		buildingAiArray->first_free_building = buildingAi;
	}

	// Hooked to add type check - Pr0nogo
	s32 remove_worker_or_building_ai(Unit* unit, UnitAi* unitAI) // 00434C90
	{
		helper::ai_remove_from_towns(unit);
		UnitAi* unitAIVar = (UnitAi*)unit->unit_ai;
		if (unitAIVar)
		{
			u16 unit_id = unit->id;
			u32 baseProperty = units_dat::base_property[unit_id];
			AiTown* town = nullptr;
			if (!unitAI)
			{
				if (unitAIVar->type == AiType::Building
					&& (baseProperty & UnitProperty::Building && unit_id != UnitId::ResourceVespeneGeyser
					|| unit_id == UnitId::ZergLarva
					|| scbw::is_larval_egg(unit)
					|| scbw::unit_id_is_zerg_supply(unit_id)))
				{
					BuildingAi* buildingAIVar = (BuildingAi*)unitAIVar;
					town = buildingAIVar->town;
					if (town)
					{
						helper::delete_building_ai(buildingAIVar, &town->free_buildings);
					}
				}
				else if (unitAIVar->type == AiType::Worker && baseProperty & UnitProperty::Worker)
				{
					WorkerAi* workerAIVar = (WorkerAi*)unitAIVar;
					town = workerAIVar->town;
					if (town)
					{
						helper::delete_worker_ai(workerAIVar, &town->free_workers);
						town->worker_count -= 1;
					}
				}
				else
				{
					return 0;
				}
			}

			if (town)
			{
				if (unit == town->building_SCV)
				{
					town->building_SCV = nullptr;
				}
				if (unit == town->main_building)
				{
					town->main_building = nullptr;
				}
				if (!unitAI)
				{
					helper::delete_ai_town_if_needed(town);
				}
			}
			unit->unit_ai = nullptr;
			return 1;
		}

		if (unit->unit_ai)
		{
			return 1;
		}
		return 0;
	}

	// Hooked to investigate unit AI corruption - Pr0nogo
	void ai_training_overlord(Unit* newUnit, Unit* oldUnit) // 00435770
	{
		if (playerTable[oldUnit->player].type == PlayerType::Computer)
		{
			UnitAi* oldUnitAI = (UnitAi*)oldUnit->unit_ai;
			if (oldUnitAI)
			{
				AiTown* oldUnitTown;
				if (oldUnitAI->type == AiType::Worker)
				{
					WorkerAi* oldWorkerAi = (WorkerAi*)oldUnit->unit_ai;
					oldUnitTown = oldWorkerAi->town;
				}
				else
				{
					BuildingAi* oldBuildingAi = (BuildingAi*)oldUnit->unit_ai;
					oldUnitTown = oldBuildingAi->town;
				}

				if (oldUnit == newUnit)
				{
					helper::ai_remove_from_towns(newUnit);
					UnitAi* newUnitAI = (UnitAi*)newUnit->unit_ai;
					if (newUnitAI)
					{
						BuildingAi* newBuildingAi = (BuildingAi*)newUnit->unit_ai;
						AiTown* newUnitTown = newBuildingAi->town;
						delete_building_ai(newBuildingAi, newUnitTown->free_buildings);
						if (newUnitTown)
						{
							if (newUnit == newUnitTown->building_SCV)
							{
								newUnitTown->building_SCV = nullptr;
							}
							if (newUnit == newUnitTown->main_building)
							{
								newUnitTown->main_building = nullptr;
							}
						}
						newUnit->unit_ai = nullptr;
					}
				}

				helper::add_worker_structure_ai(newUnit, oldUnitTown);
				u16 newUnitId = newUnit->id;
				if (newUnitId != UnitId::TerranBunker || *elapsed_time_seconds <= 60)
				{
					if (newUnitId == UnitId::TerranWatchdog)
					{
						u32 i;
						BuildingAi* j;
						s32 queuedCount;
						Unit* oldFirstTownUnit = (Unit*)oldUnitTown->workers;
						for (i = 0; oldFirstTownUnit; i += queuedCount)
						{
							Unit* oldFirstTownMoveTarget = oldFirstTownUnit->move_target.unit;
							if (oldFirstTownMoveTarget->id == UnitId::TerranWatchdog)
							{
								queuedCount = 1;
							}
							else
							{
								queuedCount = helper::ai_get_queued_count(oldFirstTownMoveTarget, UnitId::TerranWatchdog);
							}
							oldFirstTownUnit = oldFirstTownUnit->link.prev;
						}

						Unit* iterateUnit;
						s32 queuedCount2;
						for (j = oldUnitTown->buildings; j; i += queuedCount2)
						{
							iterateUnit = j->parent;
							if (iterateUnit->id == UnitId::TerranWatchdog)
							{
								queuedCount2 = 1;
							}
							else
							{
								queuedCount2 = helper::ai_get_queued_count(iterateUnit, UnitId::TerranWatchdog);
							}
							j = j->link.prev;
						}
						if (i >= 4)
						{
							helper::prepare_guard_ai(newUnit->sprite->position.y, newUnit->player, 1, newUnit->sprite->position.x);
						}
					}
				}
				else
				{
					helper::prepare_guard_ai(newUnit->sprite->position.y, newUnit->player, 5, newUnit->sprite->position.x);
					helper::prepare_guard_ai(newUnit->sprite->position.y, newUnit->player, 1, newUnit->sprite->position.x);
				}
			}
		}
	}

	// Hooked in case we need it for Lakizilisk tactical AI
	void ai_order_lurker_burrow(Unit* unit) // 0043C230
	{
		if (unit->id == UnitId::ZergLakizilisk && !(unit->status & UnitStatus::Burrowed) && unit->main_order_id != OrderId::Burrow)
		{
			helper::issue_order_targeting_nothing(unit, OrderId::Burrow);
		}
	}
	
	UnitAi* __stdcall find_nearest_military_or_guard_ai(AiRegion *ai_region, u32 a2, u32 a3, u32 x1, u32 y1, u32 a6)
	{
		MilitaryAi *result; // eax
		AiRegion *ai_region_1; // edi
		Unit *unit; // esi
		unsigned int distance; // edx
		MilitaryAi *unit_ai; // ecx
		char ai_type; // al
		char region_type; // al
		AiRegion *unit_ai_region; // ebx
		__int16 unit_ai_region_id; // di
		char unit_region_type; // al
		MilitaryAi *best_ai; // [esp+Ch] [ebp-8h]
		unsigned int min_distance; // [esp+10h] [ebp-4h]

		ai_region_1 = ai_region;
		unit = first_player_unit->unit[ai_region->player];
		result = 0;
		min_distance = 9999999;
		best_ai = 0;
		if (unit)
		{
			while (1)
			{
				if (!helper::is_valid_for(a3, unit, 0))
				{
					goto next;
				}
				distance = scbw::get_distance_fast(x1, unit->sprite->position.x, y1, unit->sprite->position.y);
				if ( distance >= min_distance )
				{
					goto next;
				}
				unit_ai = (MilitaryAi*)unit->unit_ai;
				if (!unit_ai)
				{
					goto next;
				}
				ai_type = unit_ai->type;
				if (ai_type != 1)
				{
					if (ai_type == 4)
					{
						unit_ai_region = unit_ai->region;
						unit_ai_region_id = unit_ai_region->region;
						if (unit_ai_region->region != ai_region->region)
						{
							if (unit_ai->type != 4 ||
								(unit_region_type = unit_ai_region->state, unit_region_type != 1) &&
								unit_region_type != 2 ||
								AIScriptController[ai_region->player].AI_Flags.is_use_map_settings)
							{
								if ( (!(unit->status & UnitStatus::InBuilding)
									|| a2 != 20
									&& (AIScriptController[unit->player].AI_Flags.is_use_map_settings
										|| helper::region_is_for_attack_prepare(ai_region)))
									&& (ai_region->state != 7 && !helper::region_is_for_attack_prepare(ai_region) || unit_ai_region_id != a6)
									&& unit_ai_region->defense_priority_add < a2 )
								{
									best_ai = unit_ai;
									min_distance = distance;
								}
							}
						}
						ai_region_1 = ai_region;
					}
					goto next;
				}
				if (AIScriptController[ai_region_1->player].AI_Flags.is_use_map_settings)
				{
					if (ai_region_1->air_target == (Unit*)7)
					{
						goto next;
					}
				}
				else
				{
					region_type = ai_region_1->state;
					if ( region_type != 8 && region_type != 9 )
					{
						goto next;
					}
				}
				best_ai = (MilitaryAi*)unit->unit_ai;
				min_distance = distance;

			next:
				unit = unit->player_link.next;
				if ( !unit )
				{
					return (UnitAi*)best_ai;
				}
			}
		}
		return (UnitAi*)result;
	}

	BuildingAi* create_building_ai(BuildingAiArray* ai_array) { // 004044C0
		BuildingAi* result = ai_array->first_free_building;
		if (!result) {
			return 0;
		}
		else {
			result->link.next = 0;
		}
		result = result->link.next;
		return result;
	}
}