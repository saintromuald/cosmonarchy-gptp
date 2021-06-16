#include "unit_ai.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"

namespace hooks
{
	void AI_TrainingUnit(Unit* result, Unit* parent) // 004A2830
	{
		if (result == nullptr)
		{
			return;
		}

		// archon and mind tyrant should be moved to ai_trainingnormal, once it's hooked and archon's hardcode is removed
		bool useSimpleMilitaryAI =
			result->id == UnitId::ZergRilirokor
			|| result->id == UnitId::ZergRililisk
			|| result->id == UnitId::ProtossArchon
			|| result->id == UnitId::ProtossMindTyrant;

		if (parent == nullptr)
		{
			if (useSimpleMilitaryAI)
			{
				parent = result; // in this context, parent can be the same as result and will still work as intended
			}
			else
			{
				return;
			}
		}

		if (result->playerId != parent->playerId
			|| playerTable[parent->playerId].type != PlayerType::Computer)
		{
			return;
		}

		if (useSimpleMilitaryAI)
		{
			hooks::AI_TrainingSimpleMilitary(result, parent);
		}
		else if (units_dat::BaseProperty[result->id] & (UnitProperty::Worker | UnitProperty::Building))
		{
			helper::AI_TrainingWorker(result, parent);
		}
		else if (scbw::unitIdIsZergSupply(result->id))
		{
			helper::AI_TrainingOverlord(result, parent); // replace with hook later
		}
		else
		{
			if (result->id == UnitId::TerranCleric)
			{
				if (!(AIScriptController[result->playerId].mainMedic))
				{
					AIScriptController[result->playerId].mainMedic = result;
				}
			}
			helper::AI_TrainingNormal(result, parent);
		}
	}

	// Hooked to add region null check - Pr0nogo
	void AI_TrainingSimpleMilitary(Unit* result, Unit* parent) // 0043E280
	{
		if (units_dat::BaseProperty[result->id] & (UnitProperty::Worker | UnitProperty::Building))
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

		s32 regionId = helper::SAI_GetRegionIdFromPxEx(pX, pY);
		AiRegion* playerRegion = AiRegionCaptains[parent->playerId];
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
				helper::AI_AssignMilitary(1, region);
			}
			helper::AI_AddMilitary(region, result, 1);
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	char AI_TrainingWorker(Unit* newUnit, Unit* otherUnit) // 00435700
	{
		WorkerAi* otherAI = (WorkerAi*)otherUnit->pAI;
		if (otherAI != nullptr)
		{
			if (otherUnit == newUnit)
			{
				helper::AI_RemoveFromTowns(newUnit);
				WorkerAi* newAI = (WorkerAi*)newUnit->pAI;
				if (newAI != nullptr)
				{
					AiTown* newUnitTown = newAI->town;
					if (newUnitTown != nullptr)
					{
						helper::deleteBuildingAI((BuildingAi*)newAI, &newUnitTown->free_buildings); // moved within nullcheck for sanity
						if (newUnit == newUnitTown->building_scv)
						{
							newUnitTown->building_scv = 0;
						}
						if (newUnit == newUnitTown->main_building)
						{
							newUnitTown->main_building = 0;
						}
					}
					newUnit->pAI = nullptr;
				}
			}

			AiTown* otherUnitTown = otherAI->town;
			if (otherUnitTown != nullptr) // new
			{
				return helper::AI_AddWorkerStructureAI(newUnit, otherUnitTown);
			}
		}

		return 0;
	}

	// Hooked to investigate crashes and remove Archon hardcode - Pr0nogo
	void AI_TrainingNormal(Unit* newUnit, Unit* parentUnit) // 00435DB0
	{
		s16 unitId = newUnit->id;
		if (!scbw::isConventionalUnitMorphResult(unitId))
		{
			if (newUnit->status & UnitStatus::IsHallucination)
			{
				helper::AI_AddMilitary(&AiRegionCaptains[(u8)newUnit->playerId][helper::SAI_GetRegionIdFromPxEx(newUnit->sprite->position.x, newUnit->sprite->position.y)], newUnit, 1);
				return;
			}
			if (unitId != UnitId::TerranNuclearMissile && unitId != UnitId::ProtossWitness)
			{
				BuildingAi* parentUnitAi = (BuildingAi*)parentUnit->pAI;
				BuildingAi* parentUnitAiBackup = (BuildingAi*)parentUnit->pAI;
				if (parentUnitAi)
				{
					s32 queueSlot = 0;
					if (parentUnit == newUnit)
					{
						queueSlot = 0;
						helper::AI_RemoveFromTowns(newUnit);
						BuildingAi* newUnitAi = (BuildingAi*)newUnit->pAI;
						if (newUnitAi)
						{
							AiTown* newUnitTown = newUnitAi->town;
							helper::deleteBuildingAI(newUnitAi, &newUnitTown->free_buildings);
							if (newUnitTown)
							{
								if (newUnit == newUnitTown->building_scv)
								{
									newUnitTown->building_scv = 0;
								}
								if (newUnit == newUnitTown->main_building)
								{
									newUnitTown->main_building = 0;
								}
							}
							parentUnitAi = parentUnitAiBackup;
							newUnit->pAI = 0;
						}
					}
					else if (parentUnit->id == unitId)
					{
						queueSlot = 0;
					}
					else
					{
						queueSlot = (u8)parentUnit->buildQueueSlot;
					}
					AiRegion* region = (AiRegion*)parentUnitAi->trainQueueValues[queueSlot];
					if (parentUnitAi->trainQueueTypes[queueSlot] == 2)
					{
						if (!region->neededGroundStrength)
						{
							s16 regionFullAir = region->fullAirStrength;
							s16 regionEnemyAir = region->enemyAirStrength;
							region->neededGroundStrength = (u16)newUnit;
							region->regionAirStrength = regionFullAir;
							region->fullGroundStrength = regionEnemyAir;
							newUnit->pAI = (void*)region;
							return;
						}
						parentUnitAi->trainQueueTypes[queueSlot] = 1;
						region = helper::getAiRegionInfoFromUnitLoc(newUnit);
						parentUnitAi->trainQueueValues[queueSlot] = (void*)region;
					}
					if (parentUnitAi->trainQueueTypes[queueSlot] == 1)
					{
						helper::AI_AddMilitary(region, newUnit, 0);
					}
					else
					{
						helper::AI_AddGuardAiToUnit(newUnit);
					}
				}
			}
		}
	}

	// Hooked because iquare told me to - Pr0nogo
	char AI_AddWorkerStructureAI(Unit* unit, AiTown* town) // 00433DD0
	{
		UnitAi* unitAI;
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			WorkerAiArray* workerAIArray = town->free_workers;
			unitAI = (UnitAi*)(workerAIArray->first_free_worker);
			if (unitAI != nullptr)
			{
				UnitAi* previous1 = unitAI->link.prev;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (previous1 != nullptr)
				{
					previous1->link.next = nullptr;
				}
				if (unitAI != nullptr)
				{
					unitAI->link.next = 0;
					unitAI->link.prev = (UnitAi*)town->firstTownUnit;
					if (town->firstTownUnit != nullptr && town->firstTownUnit->link.next != nullptr)
					{
						town->firstTownUnit->link.next->pAI = (WorkerAi*)unitAI;
					}
					town->firstTownUnit->pAI = (WorkerAi*)unitAI;
					WorkerAi* unitAIWorker = (WorkerAi*)unitAI;
					unitAIWorker->type = 2;
					unitAIWorker->parent = unit;
					unitAIWorker->town = town;
					unitAIWorker->target_resource = 1;
					unitAIWorker->last_update_second = *elapsedTimeSeconds;
					unitAIWorker->waitTimer = 0;
					unitAIWorker->reassign_count = 0;
					unit->beacon.flagSpawnFrame = 0;
					unit->pAI = unitAI;
					++town->worker_count;
				}
			}
		}
		else if ((units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->id != UnitId::ResourceVespeneGeyser)
			|| unit->id == UnitId::ZergLarva
			|| unit->id == UnitId::ZergEgg
			|| scbw::unitIdIsZergSupply(unit->id))
		{
			unitAI = helper::AI_CreateBuildingAi(town->free_buildings);
			if (unitAI != nullptr)
			{
				unitAI->type = AiType::Building;
				BuildingAi* unitAIBuilding = (BuildingAi*)unitAI;
				unitAIBuilding->parent = unit;
				unitAIBuilding->town = town;
				u32* buildingQueueValues = (u32*)unitAIBuilding->trainQueueValues;
				for (s32 i = 0; i < 5; i++)
				{
					buildingQueueValues[i] = 0;
				}

				u8* buildingQueueTypes = &unitAIBuilding->trainQueueTypes[0];
				for (s32 i = 0; i < 5; i++)
				{
					buildingQueueTypes[i] = 0;
				}

				unit->pAI = unitAI;
				if (unit->id == UnitId::ZergHatchery
					|| unit->id == UnitId::ZergLair
					|| unit->id == UnitId::ZergHive
					|| unit->id == UnitId::ZergSire)
				{
					if (unit->status & UnitStatus::Completed)
					{
						helper::orderComputerClear(unit, OrderId::ComputerAI);
						helper::setSecondaryOrder(unit, OrderId::SpreadCreep);
					}
				}
				if (!(AIScriptController[town->player].AI_Flags.isUseMapSettings)
					|| unit->status & UnitStatus::GroundedBuilding
					&& unit->id != UnitId::TerranWatchdog
					&& unit->id != UnitId::TerranBunker
					&& unit->id != UnitId::TerranIonCannon
					&& unit->id != UnitId::ZergCreepColony
					&& unit->id != UnitId::ZergSporeColony
					&& unit->id != UnitId::ZergSunkenColony
					&& unit->id != UnitId::ProtossPylon
					&& unit->id != UnitId::ProtossPhotonCannon)
				{
					helper::AI_MakeGuard(town->player, unit);
				}

				if (!town->inited)
				{
					town->inited = 1;
					town->mineral = 0;
					for (s32 i = 0; i < 3; i++)
					{
						town->gas_buildings[i] = 0;
					}
				}

				s32 playerID = unit->playerId;
				s16 unitType;
				s8 playerRACE = playerTable[playerID].race;
				if (playerRACE == RaceId::Terran)
				{
					unitType = UnitId::TerranCommandCenter;
				}
				else if (playerRACE == RaceId::Protoss)
				{
					unitType = UnitId::ProtossNexus;
				}
				else
				{
					unitType = UnitId::ZergHatchery;
				}

				if (unit->id == unitType || helper::unitIsActiveResourceDepot(unit))
				{
					unitAI = (UnitAi*)town->main_building->pAI;
					if (!unitAI)
					{
						town->main_building = unit;
					}
				}
				else
				{
					auto gas_structure_id = helper::getPlayerDefaultRefineryUnitType(playerID);
					if (unit->id == gas_structure_id || unit->id == UnitId::ResourceVespeneGeyser)
					{
						if (town->gas_buildings[0])
						{
							if (town->gas_buildings[1])
							{
								if (!town->gas_buildings[2])
								{
									town->gas_buildings[2] = unit;
								}
							}
							else
							{
								town->gas_buildings[1] = unit;
							}
						}
						else
						{
							town->gas_buildings[0] = unit;
						}
						if (town->resource_area)
						{
							town->resource_area_not_set = 1;
						}
					}
				}
			}
		}
		return (char)unit->id;
	}

	// Hooked to rule out AI crashes - Pr0nogo
	void AI_CreateNuke(Unit* silo, Unit* result)	// 0045B7A0
	{												// originally had 3 args; last one was unused
		if (silo->id == UnitId::TerranNuclearSilo && !*(s32*)&silo->building.silo.isReady)
		{
			Unit* nuke = helper::createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (u8)silo->playerId);
			Unit* nuke2 = nuke;
			if (nuke)
			{
				helper::updateUnitStatsFinishBuilding(nuke);
				helper::updateUnitStrengthAndApplyDefaultOrders(nuke2);
				helper::hideUnit(nuke2);
				silo->building.silo.nuke = nuke2;
				silo->building.silo.isReady = 1;
				nuke2->connectedUnit = silo;
				AI_TrainingUnit(nuke2, silo);
			}
		}
	}


	// Hooked to add functionality - Pr0nogo
	void AI_AddMilitary(AiRegion* region, Unit* unit, s32 alwaysThisRegion) // 0043DA20
	{
		// I assumed medicAI was UnitAI* but IDA tries to assign town values to it later
		// Looking at the aise version of this hook didn't help as it's more akin to a rewrite: https://pastebin.com/raw/8xNTUCuv

		/*
		if (region->state == 3 && !alwaysThisRegion)
		{
			region = &AiRegionCaptains[(u8)unit->playerId][helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y)];
		}

		Unit* medic = region->mainMedic;
		UnitAi* medicAI = (UnitAi*)medic->pAI;
		if (medicAI)
		{
			UnitAi* nextAI = (UnitAi*)medicAI->link.prev;
			medic->pAI = (void*)medicAI->link.prev;
			if (!medicAI->link.prev)
			{
				nextAI->link.next = 0;
			}

			if (nextAI)
			{
				nextAI->link.next = 0;
				nextAI->link.prev = region->town;
				AiTown* town = (AiTown*)region->town;
				if (town)
				{
					town->link.next = (AiTown*)medicAI;
				}

				region->town = (AiTown*)medicAI;
				medicAI->type = 4;
				medicAI->parent = unit;
				medicAI->captain = region;
				if (!(unit->status & 4))
				{
					*(s32*)&region->unknown_0xA += 1;
				}

				unit->pAI = medicAI;
				helper::AI_OrderToDestination(unit, unit->id != UnitId::TerranCleric ? 157 : 177,
					SAI_Paths->regions[(u16)region->region].rgnCenterX >> 8, SAI_Paths->regions[(u16)region->region].rgnCenterY >> 8);
				if (region->state == 8 || region->state == 9 || region->state == 1 || region->state == 2)
				{
					helper::AI_AssignCaptainToSlowestUnit(region);
				}
			}
		}
		*/
	}

	// Hooked to change guard assignments for structures based on AI personality -- Pr0nogo
	// TODO: Cleanup and make changes
	void AI_IterateBuildingAI(Unit* result, Unit* parent) // 00435770
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

		if (playerTable[(u8)parent->playerId].type == PlayerType::Computer)
		{
			parentAI = (UnitAi*)parent->pAI;
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
					helper::AI_RemoveFromTowns(result);
					removeBuilding = (BuildingAi*)result->pAI;
					if (removeBuilding)
					{
						removeBuildingTown = removeBuilding->town;
						helper::deleteBuildingAI(removeBuilding, &removeBuildingTown->free_buildings);
						if (removeBuildingTown)
						{
							if (result == removeBuildingTown->building_scv)
							{
								removeBuildingTown->building_scv = 0;
							}
							if (result == removeBuildingTown->main_building)
							{
								removeBuildingTown->main_building = 0;
							}
						}
						result->pAI = 0;
					}
				}
				helper::AI_AddWorkerStructureAI(result, parentTown);
				if (result->id != UnitId::TerranBunker || *elapsedTimeSeconds <= 60)
				{
					if (result->id == UnitId::TerranWatchdog)
					{
						workerUnit = parentTown->firstTownUnit;
						for (i = 0; workerUnit; i += queueCount)
						{
							workerTarget = workerUnit->moveTarget.unit;
							if (workerTarget->id == UnitId::TerranWatchdog)
							{
								queueCount = 1;
							}
							else
							{
								queueCount = helper::AI_GetQueuedCount(workerTarget, UnitId::TerranWatchdog);
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
								queueCount2 = helper::AI_GetQueuedCount(builderUnit, UnitId::TerranWatchdog);
							}
							j = j->link.prev;
						}
						if (i >= 4)
						{
							helper::AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 1, result->sprite->position.x);
						}
					}
				}
				else
				{
					helper::AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 5, result->sprite->position.x);
					helper::AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 1, result->sprite->position.x);
				}
			}
		}
	}

}