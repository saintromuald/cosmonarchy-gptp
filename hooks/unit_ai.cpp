#include "unit_ai.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "logger.h"

namespace hooks
{
	void AI_TrainingUnit(Unit* result, Unit* parent, b32 preferSimpleMilitary) // 004A2830
	{
		if (!result
			|| playerTable[result->playerId].type != PlayerType::Computer)
		{
			return;
		}

		b32 isValidParent = parent && result->playerId == parent->playerId;

		// Worker or Ovileth
		if (isValidParent)
		{
			if (units_dat::BaseProperty[result->id] & (UnitProperty::Worker | UnitProperty::Building))
			{
				helper::AI_TrainingWorker(result, parent);
				return;
			}
			if (scbw::unitIdIsZergSupply(result->id))
			{
				helper::AI_TrainingOverlord(result, parent); // replace with hook later
				return;
			}
		}

		// Military
		if (result->id == UnitId::ZergRilirokor
			|| result->id == UnitId::ZergRililisk
			|| result->id == UnitId::ProtossArchon
			|| result->id == UnitId::ProtossMindTyrant
			|| preferSimpleMilitary)
			// archon and mind tyrant should be moved to ai_trainingnormal, once it's hooked and archon's hardcode is removed
		{
			hooks::AI_TrainingSimpleMilitary(result, result);
			return;
		}
		
		// Military or Guard
		if (isValidParent)
		{
			if (result->id == UnitId::TerranCleric)
			{
				if (!(AIScriptController[result->playerId].mainMedic))
				{
					AIScriptController[result->playerId].mainMedic = result;
				}
			}
			helper::AI_TrainingNormal(result, parent);
			return;
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
	void AI_TrainingWorker(Unit* newUnit, Unit* otherUnit) // 00435700
	{
		WorkerAi* otherAI = (WorkerAi*)otherUnit->pAI;
		if (otherAI)
		{
			if (otherUnit == newUnit)
			{
				helper::AI_RemoveFromTowns(newUnit);
				WorkerAi* newAI = (WorkerAi*)newUnit->pAI;
				if (newAI)
				{
					AiTown* newUnitTown = newAI->town;
					if (newUnitTown && newAI->type == AiType::Building) // type check is new
					{
						helper::deleteBuildingAI((BuildingAi*)newAI, &newUnitTown->freeBuildings); // moved within nullcheck for sanity
						if (newUnit == newUnitTown->buildingSCV)
						{
							newUnitTown->buildingSCV = nullptr;
						}
						if (newUnit == newUnitTown->mainBuilding)
						{
							newUnitTown->mainBuilding = nullptr;
						}
					}
					newUnit->pAI = nullptr;
				}
			}

			if (otherAI->town) // new
			{
				helper::AI_AddWorkerStructureAI(newUnit, otherAI->town);
			}
		}
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
							helper::deleteBuildingAI(newUnitAi, &newUnitTown->freeBuildings);
							if (newUnitTown)
							{
								if (newUnit == newUnitTown->buildingSCV)
								{
									newUnitTown->buildingSCV = 0;
								}
								if (newUnit == newUnitTown->mainBuilding)
								{
									newUnitTown->mainBuilding = 0;
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
			WorkerAiArray* workerAIArray = town->freeWorkers;
			unitAI = (UnitAi*)(workerAIArray->first_free_worker);
			if (unitAI)
			{
				UnitAi* previous1 = unitAI->link.prev;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (previous1)
				{
					previous1->link.next = nullptr;
				}

				if (unitAI)
				{
					unitAI->link.next = 0;
					unitAI->link.prev = (UnitAi*)town->firstTownUnit;
					if (town->firstTownUnit && town->firstTownUnit->link.next)
					{
						town->firstTownUnit->link.next->pAI = (void*)unitAI;
					}
					town->firstTownUnit->pAI = (void*)unitAI;
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
					++town->workerCount;
				}
			}
		}
		else if ((units_dat::BaseProperty[unit->id] & UnitProperty::Building && unit->id != UnitId::ResourceVespeneGeyser)
			|| unit->id == UnitId::ZergLarva
			|| unit->id == UnitId::ZergEgg
			|| scbw::unitIdIsZergSupply(unit->id))
		{
			unitAI = helper::AI_CreateBuildingAi(town->freeBuildings);
			if (unitAI)
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
						town->gasBuildings[i] = 0;
					}
				}

				s32 playerID = unit->playerId;
				// Originally checked for race-specific town centers
				if (unit->id == scbw::unitIdIsTownCenter(unit->id) || helper::unitIsActiveResourceDepot(unit))
				{
					if (!town->mainBuilding->pAI)
					{
						town->mainBuilding = unit;
					}
				}
				else
				{
					u32 gas_structure_id = helper::getPlayerDefaultRefineryUnitType(playerID);
					if (unit->id == gas_structure_id || unit->id == UnitId::ResourceVespeneGeyser)
					{
						if (town->gasBuildings[0])
						{
							if (town->gasBuildings[1])
							{
								if (!town->gasBuildings[2])
								{
									town->gasBuildings[2] = unit;
								}
							}
							else
							{
								town->gasBuildings[1] = unit;
							}
						}
						else
						{
							town->gasBuildings[0] = unit;
						}

						if (town->resarea)
						{
							town->resareaNotSet = 1;
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
		if (silo->id == UnitId::TerranNuclearSilo && !silo->building.silo.isReady)
		{
			Unit* nuke = helper::createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (u8)silo->playerId);
			if (nuke)
			{
				helper::updateUnitStatsFinishBuilding(nuke);
				helper::updateUnitStrengthAndApplyDefaultOrders(nuke);
				helper::hideUnit(nuke);
				silo->building.silo.nuke = nuke;
				silo->building.silo.isReady = 1;
				nuke->connectedUnit = silo;
				AI_TrainingUnit(nuke, silo);
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
						helper::deleteBuildingAI(removeBuilding, &removeBuildingTown->freeBuildings);
						if (removeBuildingTown)
						{
							if (result == removeBuildingTown->buildingSCV)
							{
								removeBuildingTown->buildingSCV = 0;
							}
							if (result == removeBuildingTown->mainBuilding)
							{
								removeBuildingTown->mainBuilding = 0;
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

	// Hooked to investigate crashes - Pr0nogo
	void AI_RemoveUnitAI(Unit* unit, UnitAi* unitAI) // 004A1E50
	{
		helper::AI_RemoveFromAIStructs(unit, unitAI);
		helper::AI_RemoveWorkerOrBuildingAI(unit, nullptr);
		if (unitAI != nullptr)
		{
			if (unitAI->type == AiType::Guard)
			{
				GuardAi* guardAI = (GuardAi*)unitAI;
				u8 timesDied = guardAI->timesDied;
				guardAI->parent = nullptr;
				if ((u8)timesDied < 100u)
				{
					guardAI->timesDied = timesDied + 1;
				}
			}
		}
		helper::AI_MedicRemove(unit);
		unit->pAI = nullptr;
	}

	// Hooked to investigate crashes - Pr0nogo
	void deleteWorkerAI(WorkerAi* workerAi, WorkerAiArray* workerAiArray) // 00404470
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
	void deleteBuildingAI(BuildingAi* buildingAi, BuildingAiArray* buildingAiArray) // 00404500
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
	s32 AI_RemoveWorkerOrBuildingAI(Unit* unit, UnitAi* unitAI) // 00434C90
	{
		helper::AI_RemoveFromTowns(unit);
		UnitAi* unitAIVar = (UnitAi*)unit->pAI;
		if (unitAIVar)
		{
			u16 unitId = unit->id;
			u32 baseProperty = units_dat::BaseProperty[unitId];
			AiTown* town = nullptr;
			if (!unitAI)
			{
				if (unitAIVar->type == AiType::Building
					&& (baseProperty & UnitProperty::Building && unitId != UnitId::ResourceVespeneGeyser
					|| unitId == UnitId::ZergLarva
					|| scbw::isLarvalEgg(unit)
					|| scbw::unitIdIsZergSupply(unitId)))
				{
					BuildingAi* buildingAIVar = (BuildingAi*)unitAIVar;
					town = buildingAIVar->town;
					if (town)
					{
						helper::deleteBuildingAI(buildingAIVar, &town->freeBuildings);
					}
				}
				else if (unitAIVar->type == AiType::Worker && baseProperty & UnitProperty::Worker)
				{
					WorkerAi* workerAIVar = (WorkerAi*)unitAIVar;
					town = workerAIVar->town;
					if (town)
					{
						helper::deleteWorkerAI(workerAIVar, &town->freeWorkers);
						town->workerCount -= 1;
					}
				}
				else
				{
					return 0;
				}
			}

			if (town)
			{
				if (unit == town->buildingSCV)
				{
					town->buildingSCV = nullptr;
				}
				if (unit == town->mainBuilding)
				{
					town->mainBuilding = nullptr;
				}
				if (!unitAI)
				{
					helper::deleteAITownIfNeeded(town);
				}
			}
			unit->pAI = nullptr;
			return 1;
		}

		if (unit->pAI)
		{
			return 1;
		}
		return 0;
	}
}