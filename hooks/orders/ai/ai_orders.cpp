#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

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

	/*
	void AI_AddMilitary(AiRegion* region, Unit* unit, s32 alwaysThisRegion) // 0043DA20
	{
		// alwaysThisRegion acts as a bool, should be b32?
		// I assumed medicAI was UnitAI* but IDA tries to assign town values to it later
		// Looking at the aise version of this hook didn't help as it's more akin to a rewrite: https://pastebin.com/raw/8xNTUCuv
			AiRegion* regionNew;
			UnitAi* medicAI;
			Unit* medic;
			UnitAi* nextAI;
			bool previousBool;
			AiTown* AItown;

			regionNew = region;
			if (region->state == 3 && !alwaysThisRegion) {
				regionNew = &AiRegionCaptains[(u8)unit->playerId][SAI_GetRegionIdFromPxEx(
					unit->sprite->position.x,
					unit->sprite->position.y)];
			}
			medic = region->mainMedic;
			medicAI = (UnitAi*)medic->pAI;
			if (medicAI) {
				nextAI = (UnitAi*)medicAI->link.prev;
				medic->pAI = (void*)medicAI->link.prev;
				if (!medicAI->link.prev)
					nextAI->link.next = 0;
				if (nextAI) {
					nextAI->link.next = 0;
					nextAI->link.prev = regionNew->town;
					AItown = (AiTown*)regionNew->town;
					if (AItown)
						AItown->link.next = (town*)medicAI;
					regionNew->town = (town*)medicAI;
					medicAI->type = 4;
					medicAI->parent = unit;
					medicAI->captain = regionNew;
					if (!(unit->status & 4))
						++* (s32*)&regionNew->unknown_0xA;
					unit->pAI = medicAI;
					AI_OrderToDestination(
						unit,
						unit->id != UnitId::TerranMedic ? 157 : 177,
						SAI_Paths->regions[(u16)regionNew->region].rgnCenterX >> 8,
						SAI_Paths->regions[(u16)regionNew->region].rgnCenterY >> 8);
					if (regionNew->state == 8 || regionNew->state == 9 || regionNew->state == 1 || regionNew->state == 2)
						assignCaptainToSlowestUnit(parentAi);
				}
			}
		}
		*/

	// Hooked to rule out AI crashes - Pr0nogo
	void AI_CreateNuke(Unit* silo, Unit* result)	// 0045B7A0
	{												// originally had 3 args, but last one was unused
		Unit* nuke;
		Unit* nuke2;

		if (silo->id == UnitId::TerranNuclearSilo && !*(s32*)&silo->building.silo.isReady)
		{
			nuke = helper::createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (u8)silo->playerId);
			nuke2 = nuke;
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
		char result = 0;
		WorkerAi* otherUnitAI = (WorkerAi*)otherUnit->pAI;
		if (otherUnitAI != nullptr)
		{
			AiTown* otherUnitTown = otherUnitAI->town;
			if (otherUnit == newUnit)
			{
				helper::AI_RemoveFromTowns(newUnit);
				WorkerAi* newUnitAI = (WorkerAi*)newUnit->pAI;
				if (newUnitAI)
				{
					AiTown* newUnitTown = newUnitAI->town;
					helper::deleteBuildingAi((BuildingAi*)newUnitAI, &newUnitTown->free_buildings);
					if (newUnitTown)
					{
						if (newUnit == newUnitTown->building_scv)
						{
							newUnitTown->building_scv = nullptr;
						}
						if (newUnit == newUnitTown->main_building)
						{
							newUnitTown->main_building = nullptr;
						}
					}
					newUnit->pAI = nullptr;
				}
			}
			result = helper::AI_AddWorkerStructureAI(newUnit, otherUnitTown);
		}
		return result;
	}

	// Hooked to investigate crashes and remove Archon hardcode - Pr0nogo
	void AI_TrainingNormal(Unit* newUnit, Unit* parentUnit) // 00435DB0
	{
		s16 unitId = newUnit->id;
		if (unitId != UnitId::ZergGeszithalor && unitId != UnitId::ZergVithrilisk)
		{
			if (newUnit->status & 0x40000000)
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
							helper::deleteBuildingAi(newUnitAi, &newUnitTown->free_buildings);
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
		WorkerAi* unitAIWorker;
		BuildingAi* unitAIBuilding;
		WorkerAiArray* workerAIArray;
		UnitAi* previous1;
		u32* buildingQueueValues;
		u8* buildingQueueTypes; // was char*
		s8 playerRACE;
		s32 playerID;
		u16 unitType;

		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
		{
			workerAIArray = town->free_workers;
			unitAI = (UnitAi*)(workerAIArray->first_free_worker);
			if (unitAI != nullptr)
			{
				previous1 = unitAI->link.prev;
				auto prevIsNull = unitAI->link.prev == 0;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (!prevIsNull)
				{
					previous1->link.next = 0;
				}
				if (unitAI != nullptr)
				{
					unitAI->link.next = 0;
					unitAI->link.prev = (UnitAi*)town->firstTownUnit;
					if (town->firstTownUnit)
					{
						town->firstTownUnit->link.next->pAI = (WorkerAi*)unitAI;
					}
					town->firstTownUnit->pAI = (WorkerAi*)unitAI;
					unitAIWorker = (WorkerAi*)unitAI;
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
		else if ((units_dat::BaseProperty[unit->id] & 1 && unit->id != UnitId::ResourceVespeneGeyser)
			|| unit->id == UnitId::ZergLarva
			|| unit->id == UnitId::ZergEgg
			|| unit->id == UnitId::ZergOvileth
			|| unit->id == UnitId::ZergIroleth
			|| unit->id == UnitId::ZergOthstoleth
			|| unit->id == UnitId::ZergAlaszileth)
		{
			unitAI = helper::AI_CreateBuildingAi(town->free_buildings);
			if (unitAI)
			{
				unitAI->type = 3; // building ai
				unitAIBuilding = (BuildingAi*)unitAI;
				unitAIBuilding->parent = unit;
				unitAIBuilding->town = town;
				buildingQueueValues = (u32*)unitAIBuilding->trainQueueValues;
				for (s32 i = 0; i < 5; i++)
				{
					buildingQueueValues[i] = 0;
				}
				buildingQueueTypes = &unitAIBuilding->trainQueueTypes[0];
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
				if (!(AIScriptController[(u8)town->player].AI_Flags.isUseMapSettings)
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
				playerID = unit->playerId;
				playerRACE = playerTable[playerID].race;
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

	// Don't know what this was hooked for; tidied May 2021 - Pr0nogo
	void AI_BuildTechUpgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need) // 00431F90
	{
		s32 iterate = 0;
		constexpr s32 BuildCountFlag = 0xF8;
		TownReq* townUnits;
		if (playerTable[(u8)town->player].type == PlayerType::Computer)
		{
			townUnits = town->town_units;
			bool setNeed = false;
			if (town->town_units[0].flags_and_count & BuildCountFlag) // if build
			{
				for (s32 i = 0; i < 100; i++)
				{
					++townUnits;
					if (!(townUnits->flags_and_count & BuildCountFlag)) // if not build
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
				town->town_units[iterate].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->town_units[iterate].id = id;
				town->town_units[iterate].priority = priority;
				// build orders have 0xf8 flag, tech has 0x4, upgrade has 0x2, 0x1 - only if needed
			}
		}
	}

	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp) // 0043F320
	{
		BuildingAi* selfBuildingAi;
		s32 quartTotalHP;
		Unit* target;
		UnitAi* v12;
		AiRegion* v14;
		s32 v15;
		AiRegion* selfRegion;
		char v18;
		s16 v19;
		s16 child0;
		s32 child1;
		s32 child3;
		s32 child5;
		s32 child6;
		Unit* child7;
		__int64 parentAi3;
		Unit* parentAi4;
		Unit* parentAi6;
		Unit* parentAi7;
		s32 parentAi9;
		s32 childAi0;
		s32 childAi1;
		s32 childAi2;
		Unit* realTarget;
		s32 childAi4;
		MilitaryAi* militaryAi;

		if (playerTable[self->playerId].type != PlayerType::Computer || self->playerId >= 8u)
		{
			return;
		}
		if (units_dat::BaseProperty[(u16)self->id] & UnitProperty::Building)
		{
			if (self->pAI)
			{
				AIScriptController[self->playerId].lastIndividualUpdateTime = *elapsedTimeSeconds;
				selfBuildingAi = (BuildingAi*)self->pAI;
				if (selfBuildingAi)
				{
					selfBuildingAi->town->in_battle = 1;
				}
				if (mainTargetReactions)
				{
					if (attacker)
					{
						if (units_dat::BaseProperty[(u16)self->id] & UnitProperty::Worker // cancel structure code
							&& !(self->mainOrderId) // Original function call checks if unit has a sprite??? 00475A50
							&& !(self->status & UnitStatus::Completed))
						{
							quartTotalHP = self->getTotalHitPointsPlusShields() / 4;
							if (self->getCurrentLifeInGame() < quartTotalHP)
							{
								helper::AI_CancelStructure(self); // originally unused variable v8
								return;
							}
						}
					}
				}
			}
		}
		target = helper::AI_GetActualTarget(self, attacker);
		realTarget = target;
		if (target->status & UnitStatus::InBuilding)
		{
			realTarget = target->connectedUnit;
			if (!realTarget)
			{
				return;
			}
		}
		if (realTarget->status & UnitStatus::CloakingForFree)
		{
			if (!(realTarget->status & UnitStatus::CloakingForFree))
			{
				realTarget = helper::checkArbiterCloakingRange(self, UnitId::ProtossDidact);
				if (!realTarget)
				{
					return;
				}
			}
		}
		if (mainTargetReactions)
		{
			helper::AI_OrderUnitCloaking(self);
			helper::AI_OrderLurkerBurrow(self);
		}
		v12 = (UnitAi*)self->pAI;
		parentAi9 = 0;
		childAi1 = 0;
		if (!v12)
		{
			goto LABEL_37;
		}
		if (v12->type == 4)
		{
			militaryAi = (MilitaryAi*)v12;
			if (militaryAi->region->state == 1)
			{
				if (self->isUnitAINonBuilding())
				{
					helper::AI_AssignMilitary(2, militaryAi->region);
				}
			}
			childAi1 = helper::AI_IsUnderAttack(self, 0);
		}
		if (!mainTargetReactions || !helper::unitIsActiveTransport(self) || !(self->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (helper::isUnitLoaded(self))
		{
			if (self->mainOrderId != 111)
			{
				helper::orderComputerClear(self, 0x6Fu);
			}
			return;
		}
		if (helper::AI_TryReturnHome(self, 0) || !helper::AI_TransportAction(self))
		{
			if (AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings)
			{
				parentAi9 = 1;
			}
LABEL_37:
			v14 = &AiRegionCaptains[(u8)self->playerId][helper::SAI_GetRegionIdFromPxEx(self->sprite->position.x, self->sprite->position.y)];
			if (!parentAi9)
			{
				if (!dontCallHelp)
				{
					helper::AI_AskForHelp(self, realTarget, childAi1);
				}
				if (!childAi1)
				{
					if (!self->pAI
						|| helper::AI_IsUnderAttack(self, 0)
						|| (childAi2 = 1, *(s8*)(v15 + 8) == 1)
						&& AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings)
					{
						childAi2 = 0;
					}
					selfRegion = &AiRegionCaptains[(u8)self->playerId][helper::getRegionIdFromUnit(realTarget)];
					if (childAi2)
					{
						v18 = v14->state;
						if (v18 == 5 || v18 == 4)
						{
							helper::AI_AssignMilitary(6, v14);
						}
						if (v14->state)
						{
							if (!selfRegion->state && *elapsedTimeSeconds > 60)
								helper::AI_AssignMilitary(5, selfRegion);
						}
					}
					if (!dontCallHelp)
					{
						selfRegion->enemyAirStrength = helper::getEnemyAirStrengthInRegion(selfRegion->region, (u8)self->playerId);
						selfRegion->enemyGroundStrength = helper::getEnemyStrengthInRegion(selfRegion->region, (u8)self->playerId, 0);
						v19 = selfRegion->neededAirStrength;
						if (v19 > v14->neededAirStrength)
						{
							v14->neededAirStrength = v19;
						}
						child0 = selfRegion->neededGroundStrength;
						if (child0 > v14->neededGroundStrength)
						{
							v14->neededGroundStrength = child0;
						}
					}
				}
			}
			v14->captainFlags |= 0x20u;
			if (self->status & UnitStatus::Completed)
			{
				child1 = (u8)self->mainOrderId;
				childAi0 = (u8)self->mainOrderId;
				if (child1 != OrderId::CompletingArchonSummon && child1 != OrderId::ResetCollision1 && child1 != OrderId::ConstructingBuilding)
				{
					if (mainTargetReactions || !scbw::orderIsSpell(child1))
					{
						child3 = (u16)self->id;
						if (self->id || child1 != OrderId::EnterTransport)
						{
							childAi4 = helper::unitIsPerformingUnbreakableCode(self);
							if (childAi4 || !helper::AI_CastSpellBehavior(self, mainTargetReactions))
							{
								if (child1 != OrderId::RechargeShields1 && child1 != OrderId::Move)
								{
									if (!mainTargetReactions)
									{
										goto LABEL_126;
									}
									if (child3 != UnitId::ProtossSolarion
//										|| (u8)(LOBYTE(self->field_C8) + BYTE1(self->carrier.inHangarCount)) > 2u
										|| !helper::AI_TransportAction(self))
									{
										if (realTarget && realTarget->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(u8)self->playerId].AI_Flags.isUseMapSettings
											|| ((child5 = 3 * self->getTotalHitPointsPlusShields() / 4, self->getCurrentLifeInGame() > child5)
												|| child3 != UnitId::ProtossDidact && child3 != UnitId::TerranWraith && child3 != UnitId::ZergMutalisk
												|| !helper::AI_TransportAction(self))
											&& ((child6 = 2 * self->getTotalHitPointsPlusShields() / 3, self->getCurrentLifeInGame() > child6)
												|| child3 != UnitId::TerranMinotaur && child3 != UnitId::ProtossSolarion && child3 != UnitId::ProtossPanoptus
												|| !helper::AI_TransportAction(self)))
										{
											if (self->id != UnitId::ZergLakizilisk
												|| self->status & UnitStatus::Burrowed
												|| childAi0 == OrderId::Burrow
												|| childAi0 == OrderId::Unburrow)
											{
												child7 = realTarget;
											}
											else
											{
												child7 = realTarget;
												if (helper::AI_Flee(self, realTarget))
												{
													return;
												}
											}
											if (childAi4
												|| !(self->status & UnitStatus::IsBuilding)
												|| helper::unitHasGroundPath(self, child7->sprite->position.x, child7->sprite->position.y)
												|| !helper::AI_Flee(self, child7))
											{
LABEL_126:
												if (self->orderTarget.unit != realTarget)
												{ // Responding with worker
													if (!(units_dat::BaseProperty[(u16)self->id] & UnitProperty::Worker)
//														|| (child9 = realTarget->id, units_dat::BaseProperty[child9] & UnitProperty::Worker)
														|| !scbw::unitIsRangedAttacker(realTarget) // previously only checked for Zergling id
														|| helper::AI_HasNoDefenseBuild((u8)self->id))
													{
														if (mainTargetReactions)
														{
															if (self->pAI)
															{
																if (helper::unitCanSeeCloakedTarget(realTarget, self))
																{
																	helper::AI_RespondToCloakedThreat(self, realTarget);
																}
															}
														}
														if (!childAi4)
														{
															if (helper::unitCanAttackTarget(realTarget, self, 1))
															{
																parentAi6 = self->subunit;
																parentAi7 = self->subunit;
																self->autoTargetUnit = realTarget;
																if (parentAi7->isSubunit())
																{
																	parentAi6->autoTargetUnit = realTarget;
																}
																helper::AI_UpdateAttackTarget(self, 0, 1, 0);
															}
															else if (self->pAI)
															{
																if (childAi0 != 159
//																	|| (parentAi3 = AI_PrepareToFlee(realTarget, self), (s32)parentAi3 == *(s32*)&self->sprite->position)
																	)
																{
																	if (mainTargetReactions)
																	{
																		if (self->id != 34)
																		{
																			parentAi4 = self->orderTarget.unit;
																			if (!parentAi4 || !helper::unitCanAttackTarget(parentAi4, parentAi4, 1))
																			{
																				helper::AI_Flee(self, realTarget);
																			}
																		}
																	}
																}
																else
																{
//																	orderInterrupt(self, 6u, (Point32*)parentAi3, (Unit*)HIDWORD(parentAi3)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													}
													else if (mainTargetReactions)
													{
														helper::AI_Flee(self, realTarget);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
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
						helper::deleteBuildingAi(removeBuilding, &removeBuildingTown->free_buildings);
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

	// hooked for attack extender
//	0x0043E670 => train_attack_force(@eax *mut AiRegion)->u32;
	u32 AI_TrainAttackForce(AiRegion* region) // 0043E670
	{
		u32 result;
		u32 child;
		AiRegion* parentAi;
		u16 childAi;
		u32 childTown;
		u32 v6;
		u32 parentTown;
		u32 v8;
		u32 v9[63];
		u32 v10;
		u32 v11;
		u32 v12;

		parentAi = region;
		child = (u8)region->playerId;
		result = AI_GetMissingAttackForceUnits(region, v9, AIScriptController[child].AI_Flags.isUseMapSettings);
		v10 = result;
		if (AIScriptController[child].AI_Flags.isUseMapSettings
			|| (childTown = AIScriptController[child].lastIndividualUpdateTime) != 0
			&& (v12 = *elapsedTimeSeconds - childTown >= 30, *elapsedTimeSeconds - childTown < 30))
		{
			v11 = 60;
		}
		else
		{
			v11 = 160;
		}
		v6 = 0;
		v12 = 0;
		if (result)
		{
			do
			{
				parentTown = v9[v6];
				if (parentTown < UnitId::None && !helper::AI_AddRegionToMilitary(parentAi, parentTown, v11))
				{
					v8 = (u8)parentAi->playerId;
					AIScriptController[v8].AI_Flags.flag_0x40 = 1; // Might be wrong
					helper::AI_FindUnit(parentTown, v8, 1);
					helper::AI_AddSpendingRequest(parentTown, v8, 1, 60, parentAi);
					helper::AI_AddBaseUnitSpendingRequest(parentTown, v8, parentAi, 60);
					v6 = v12;
				}
				result = v10;
				v12 = ++v6;
			}
			while (v6 < v10);
		}
		return result;
	}

	//	0x0043EEC0 => grouping_for_attack(*mut AiRegion);
	void AI_GroupingForAttack(AiRegion* region) // 0043EEC0
	{
		AiRegion* regionVar;
		Unit* attackTarget;
		AiRegion* targetRegion;
		u32 missingUnits;
		u32 attackUnit;
		u16 regionId;
		AiRegion* targetRegion2;
		AiRegion* targetRegion3;
		u32 attackWave[63];
		s32 a3;
		s32 a4;
		s32 a2;

		helper::AI_RecalculateRegionStrength(region);
		helper::AI_TargetEnemyInRegion(region);
		if (region->captainFlags & 0x20 && helper::AI_MoveMilitaryToDefend(region))
		{
			return;
		}
		helper::AI_GetRegionMilitaryTypes(region, &a2, &a3, &a4);
		regionVar = region;
		if (a2 && (*SAI_Paths)->regions[region->region].accessabilityFlags == 0x1FFD || helper::AI_CanTravelToRegion(region, region))
		{
			helper::AI_AssignMilitary(0, region);
			attackTarget = helper::AI_PickAttackTarget_NoBuildings(region);
			if (attackTarget)
			{
				targetRegion = &AiRegionCaptains[(u8)region->playerId][helper::SAI_GetRegionIdFromPxEx(
					attackTarget->sprite->position.x,
					attackTarget->sprite->position.y)];
				helper::AI_AssignMilitary(9, targetRegion);
				helper::AI_MoveMilitaryToOtherRegion(targetRegion, region, 0, 0, 1);
			}
			else
			{
				helper::AI_IterateTownSomething(region);
			}
			return;
		}
		if (helper::AI_AttackTimerDecrement((u8)region->playerId))
		{
LABEL_15:
			regionId = region->targetRegion;
			if (regionId)
			{
				targetRegion2 = &AiRegionCaptains[(u8)region->playerId][regionId];
				helper::AI_AssignMilitary(1, targetRegion2);
				targetRegion3 = targetRegion2;
				regionVar = region;
				helper::AI_MoveMilitaryToOtherRegion(targetRegion3, region, 0, 0, 1);
			}
			helper::AI_AssignMilitary(0, regionVar);
			return;
		}
		missingUnits = AI_GetMissingAttackForceUnits(region, attackWave, 1);
		attackUnit = 0;
		if (!missingUnits)
		{
LABEL_14:
			if (helper::AI_HasForcesMovingToAttack((u8)region->playerId))
			{
				return;
			}
			goto LABEL_15;
		}
		while ((u32)attackWave[attackUnit] >= 0xE4)
		{
			if (++attackUnit >= missingUnits)
			{
				goto LABEL_14;
			}
		}
	}

	//	0x00438050 => get_missing_attack_force_units_hook(*mut AiRegion,*mut u32,bool)->u32;
	u32 AI_GetMissingAttackForceUnits(AiRegion* region, u32* unitIds, u32 isCampaign) // 00438050
	{
		s16 parentAi;
		u16* childAi;
		u32 i;
		u32 v6;
		Unit* parentTown;
		s8 v8;
		u32 v9;
		u32 v10; // was unitIDs, not sure what it should be here
		s32 v11;
		u32 v12;
		s32 v13;
		UnitAi* v14;
		u32 v15;
		u32 v16;
		Unit* v18;
		MilitaryAi* military;

		childAi = AIScriptController[(u8)region->playerId].attackers;
		parentAi = *childAi;
		for (i = 0; *childAi; ++i)
		{
			unitIds[i] = parentAi - 1;
			++childAi;
			parentAi = *childAi;
		}
		v6 = (u8)region->playerId;
		parentTown = firstPlayerUnit->unit[v6];
		v18 = firstPlayerUnit->unit[v6];
		if (parentTown)
		{
			while (1)
			{
				if (parentTown->sprite)
				{
					v8 = parentTown->mainOrderId;
					if (v8)
					{
						v9 = (u16)parentTown->id;
						if (!(units_dat::BaseProperty[v9] & UnitProperty::Subunit))
							break;
					}
				}
LABEL_37:
				v18 = parentTown->player_link.next;
				if (!v18)
					return i;
				parentTown = parentTown->player_link.next;
			}
			v10 = (u16)parentTown->id;
			if (v9 == UnitId::TerranPhalanxSiegeMode)
				v10 = UnitId::TerranPhalanxTankMode;
			v11 = 1;
			v12 = parentTown->status & 1;
			if (!v12 || v8 == OrderId::CompletingArchonSummon || v8 == OrderId::ResetCollision1)
			{
				if (isCampaign)
				{
LABEL_36:
					parentTown = v18;
					goto LABEL_37;
				}
				if (v10 == UnitId::ZergEgg || v10 == UnitId::ZergCocoon)
				{
					v10 = (u16)v18->buildQueue[(u8)v18->buildQueueSlot];
					v13 = units_dat::BaseProperty[v10];
					if (v13 & 0x400)
						v11 = 2;
				}
			}
			if (!(units_dat::ComputerAIInternal[v10] & 1))
			{
				v14 = (UnitAi*)v18->pAI;
				military = (MilitaryAi*)v18->pAI;
				if (!v12
					|| v8 == 0x6A // UNI_FIRST_BLDG
					|| v8 == UnitId::Special_MatureChrysalis
					|| v14 != 0 && v14->type == 4 && military->region == region)
				{
					if (v11)
					{
						v15 = i - 1;
						do
						{
							v16 = 0;
							if (i)
							{
								while (v10 != unitIds[v16])
								{
									if (++v16 >= i)
									{
										goto LABEL_35;
									}
								}
								unitIds[v16] = UnitId::None;
								if (v16 == v15)
								{
									--i;
									--v15;
								}
							}
LABEL_35:
							--v11;
						}
						while (v11);
					}
				}
			}
			goto LABEL_36;
		}
		return i;
	}

	//	0x00447230 => ai_add_to_attackforce(@edi u32,@eax u32,@ecx u32);
	void AI_AddToAttack(u32 result, u32 player, u32 unitId) // 00447230
	{
		u32 counter;
		u32 index;
		u32 plOffset;
		u16* atkGroups;
		u16* atkFinal;

		counter = result;
		if (player < 8 && result < 63 && unitId < UnitId::None)
		{
			plOffset = 628 * player;
			index = 0;
			if (AIScriptController[player].attackers[0])
			{
				atkGroups = AIScriptController[player].attackers;
				while (index < 63)
				{
					++atkGroups;
					++index;
					if (!*atkGroups)
					{
						goto LABEL_8;
					}
				}
			}
			else
			{
LABEL_8:
				if (counter)
				{
					atkFinal = &AIScriptController[0].attackers[plOffset + index];
					do
					{
						--counter;
						if (index >= 63)
						{
							break;
						}
						*atkFinal = unitId + 1;
						++index;
						++atkFinal;
					}
					while (counter);
				}
			}
		}
	}

	//	0x0043ABB0 => ai_attack_to_hook(u32,i32,i32,bool,bool)->u32;
	bool AI_AttackManager(u32 player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack, u32 allowFallbackAirOnly) // 0043ABB0
	{
		u16 attackWave;
		u16* attackWave2;
		char regionsList[2500];
		u32 waveBool;

		if (AIScriptController[player].supplyCollection)
		{
			return 1;
		}
		if (AIScriptController[player].AI_AttackGroup)
		{
			return *((u8*)&AiRegionCaptains[player][(u16)AIScriptController[player].AI_AttackGroup] - 47) != 8;
		}
		memset(regionsList, 0, sizeof(regionsList));
		helper::PopulateRegionsWithOwn(regionsList, player);
		helper::PopulateRegionsWithEnemy(player, regionsList);
		while (helper::AI_GetNeighborRegionTypes(regionsList))
		{
			helper::AI_FinishRegionAccessibility(regionsList);
		}
		attackWave = AIScriptController[player].attackers[0];
		attackWave2 = AIScriptController[player].attackers;
		waveBool = attackWave != 0;
		if (attackWave)
		{
			while (units_dat::DestroyScore[2 * attackWave + 226] & 4)
			{
				attackWave = attackWave2[1];
				++attackWave2;
				if (!attackWave)
				{
					goto LABEL_12;
				}
			}
			waveBool = 0;
		}
LABEL_12:
		AIScriptController[player].AI_Flags.isSecureFinished = 1;
		if (!helper::AI_SetAttackGroupingRegion(waveBool, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
			&& !helper::AI_SetAttackGroupingRegion(waveBool, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack))
		{
			if (!allowFallbackAirOnly || waveBool)
			{
				return 1;
			}
			if (!helper::AI_SetAttackGroupingRegion(1, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
				&& !helper::AI_SetAttackGroupingRegion(1, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack))
			{
				AIScriptController[player].supplyCollection = 1;
				return 1;
			}
		}
		return 0;
	}

	//	0x00437F20 => has_units_for_attack_force_hook(@edx u32)->u32;
	s32 AI_HasUnitsForAttackForce(u32 player) // 00437F20
	{
		u16* child;
		u16 parentAi;
		u32 i;
		Unit* childTown;
		u32 v6;
		u8 parentTown;
		u32 v8;
		s32 v9;
		u32 v10;
		u32 v11;
		u32 v13[63];

		child = AIScriptController[player].attackers;
		parentAi = *child;
		for (i = 0; *child; ++i)
		{
			v13[i] = parentAi - 1;
			++child;
			parentAi = *child;
		}
		childTown = firstPlayerUnit->unit[player];
		if (childTown)
		{
			v6 = i - 1;
			do
			{
				if (childTown->sprite)
				{
					parentTown = childTown->mainOrderId;
					if (parentTown)
					{
						v8 = (u16)childTown->id;
						if (!(units_dat::BaseProperty[v8] & 0x10))
						{
							v9 = (u16)childTown->id;
							if (v8 == 30)
							{
								v9 = 5;
							}
							if (!(units_dat::ComputerAIInternal[v9] & 1))
							{
								if (childTown->status & 1)
								{
									if (parentTown != 106 && parentTown != 150 && !orders_dat::UseWeaponTargeting[parentTown])
									{
										if (childTown->pAI)
										{
											v10 = 0;
											if (i)
											{
												while (v9 != v13[v10])
												{
													if (++v10 >= i)
													{
														goto LABEL_22;
													}
												}
												v13[v10] = 228;
												if (v10 == v6)
												{
													--i;
													--v6;
												}
											}
										}
									}
								}
							}
						}
					}
				}
LABEL_22:
				childTown = childTown->player_link.next;
			}
			while (childTown);
		}
		v11 = 0;
		if (!i)
		{
			return 1;
		}
		while (v13[v11] == 228)
		{
			if (++v11 >= i)
			{
				return 1;
			}
		}
		return 0;
	}

	// Hooked to help with AI recloaking units after the cloak rewrite - Pr0nogo
	void AI_orderUnitCloaking(Unit* unit) // 0043B970
	{
		return;
		/*
		s32 unitStatus = unit->status;
		if (!(unitStatus & 0x100))
		{
			s8 techId;
			if (unit->id == UnitId::TerranEidolon)
			{
				techId = TechId::PersonnelCloaking;
			}
			else if (unit->id == UnitId::TerranWraith)
			{
				techId = TechId::CloakingField;
			}
			if (helper::techUseAllowed(unit, techId, unit->playerId) == 1)
			{
				helper::applyCloakingOrder(unit);
			}
		}
		*/
	}

	// Hooked to improve functionality - Pr0nogo
	void AI_SendSuicide(char randomSuicide, s32 player) // 0043E050
	{
		if (randomSuicide)
		{
			for (Unit* unit = firstPlayerUnit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (unit->sprite)
				{
					if (unit->mainOrderId)
					{
						if (!unit->isSubunit()
							&& unit->id != UnitId::TerranNuclearMissile
							&& unit->id != UnitId::ZergLarva
							&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
							&& !(unit->sprite->flags & SpriteFlags::Hidden)
							&& !helper::isThreat(0, unit))
						{
							helper::prepSuicide(unit);
						}
					}
				}
			}
			return;
		}

		if (!AIScriptController[player].strategic_suicide)
		{
			AiRegion* region2 = 0;

			s32 regionIndex = 2500;
			for (Unit* unit = firstPlayerUnit->unit[player]; unit; unit = unit->player_link.next)
			{
				if (!unit->sprite)
				{
					continue;
				}
				if (!unit->mainOrderId)
				{
					continue;
				}
				GuardAi* guardAI = (GuardAi*)unit->pAI;
				if (!guardAI
				  || helper::unitIsActiveTransport(unit)
				  || unit->status & UnitStatus::InBuilding && !(AIScriptController[player].AI_Flags.isUseMapSettings))
				{
					continue;
				}
				s16 regionId = helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y);
				AiRegion* region = &AiRegionCaptains[player][regionId];
				u8 aiType = guardAI->type;
				if (aiType == 1)
				{
					guardAI->parent->pAI = nullptr;
					helper::AI_AddMilitary(region, guardAI->parent, 1);
					guardAI->parent->pAI = nullptr;
				}
				else if (aiType != 4)
				{
					continue;
				}
				MilitaryAi* unitAI2 = (MilitaryAi*)unit->pAI;
				if (unitAI2)
				{
					if (unitAI2->type == 4)
					{
						MilitaryAi* militaryAI = unitAI2;
						u8 regionType = militaryAI->region->state;
						if (regionType == 1 || regionType == 8 || regionType == 9 || regionType == 2)
						{
							continue;
						}
					}
				}
				if (regionIndex != 2500)
				{
					helper::AI_RemoveUnitAI(unit, 0);
					helper::AI_AddMilitary(region2, unit, 1);
					continue;
				}
				memset(AIScriptController[player].attackers, 0, sizeof(AIScriptController[player].attackers));
				AIScriptController[player].AI_AttackGroup = 0;
				helper::AI_AttackClear(player);
				if (!AI_AttackManager(player, unit->sprite->position.x, unit->sprite->position.y, 0, 1))
				{
					AIScriptController[player].strategic_suicide = 24;
					regionIndex = (u16)AIScriptController[player].AI_AttackGroup - 1;
					region2 = &AiRegionCaptains[player][regionIndex];
					helper::AI_RemoveUnitAI(unit, 0);
					helper::AI_AddMilitary(region2, unit, 1);
				}
			}
		}
	}

	// Hooked to investigate crashes - Pr0nogo
	u32 AI_RecalculateRegionStrength(AiRegion* region) // 0043A390
	{
		const u32 Func_CB_AICaptainStrengthIterator = 0x00438F60;
		AiTown* regionTown = region->town;
		region->regionAirStrength = 0;
		region->fullAirStrength = 0;
		region->regionGroundStrength = 0;
		region->fullGroundStrength = 0;
			
		for (AiTown* i = regionTown; i; i = i->link.prev)
		{
			Unit* townUnit = i->firstTownUnit;
			if (!townUnit)
			{
				continue;
			}
			if (!(townUnit->status & UnitStatus::InBuilding))
			{
				s32 townUnitGroundStrength = townUnit->groundStrength;
				s16 townUnitAirStrength = townUnit->airStrength;
				region->fullAirStrength += townUnitAirStrength;
				region->fullGroundStrength += townUnitGroundStrength;
				Sprite* townUnitSprite = townUnit->sprite;
				if (!townUnitSprite)
				{
					continue;
				}
				s32 v6 = townUnitSprite->position.x;
				s32 regionTown6 = townUnitGroundStrength;
				if (helper::SAI_GetRegionIdFromPxEx(v6, townUnitSprite->position.y) == region->region)
				{
					s16 v7 = regionTown6;
					region->regionAirStrength += townUnitAirStrength;
					region->regionGroundStrength += v7;
				}
			}
		}

		if (AIScriptController[region->playerId].AI_Flags.isUseMapSettings)
		{
			s32 v8 = region->region << 6;
			s32 v9 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + v8);
			s32 regionTown0 = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + v8) >> 8;
			v9 >>= 8;
			s16 a1 = regionTown0 - 256;
			s32 regionTown6 = (v9 + 256) << 16;
			s16 regionTown5 = v9 - 256;
			regionTown6 |= regionTown0 + 256;
			helper::iterateUnitsAtLocationTargetProc((Box16*)&a1, (Unit*(__fastcall*)(Unit*, Unit*))Func_CB_AICaptainStrengthIterator, (Unit*)region);
		}

		u32 regionAirStrength = region->enemyAirStrength;
		if (!(AIScriptController[region->playerId].AI_Flags.isUseMapSettings))
		{
			regionAirStrength += regionAirStrength >> 1;
		}
		if (regionAirStrength > region->neededAirStrength)
		{
			region->neededAirStrength = regionAirStrength;
		}

		u32 regionGroundStrength = region->enemyGroundStrength;
		if (!(AIScriptController[region->playerId].AI_Flags.isUseMapSettings))
		{
			regionGroundStrength += regionGroundStrength >> 1;
		}
		if (regionGroundStrength > region->neededGroundStrength)
		{
			region->neededGroundStrength = regionGroundStrength;
		}

		u16 result = AIScriptController[region->playerId].AI_MaxForce;
		if (AIScriptController[region->playerId].AI_MaxForce)
		{
			if (region->neededAirStrength > result)
			{
				region->neededAirStrength = result;
			}
			if (region->neededGroundStrength > result)
			{
				region->neededGroundStrength = result;
			}
		}
		return result;
	}

	// Hooked to investigate crashes - Pr0nogo
	AiTown* AI_progressBuild(s32 playerId, s32* minerals, s32* gas) // 00434480
	{
		AiTown* result = AiActiveTownArray[playerId].first;
		AiTown* firstTown = AiActiveTownArray[playerId].first;
		if (result)
		{
			do
			{
				Unit* firstTownUnitVar = firstTown->firstTownUnit;
				if (firstTown->firstTownUnit)
				{
					do
					{
						Unit* targetUnit = firstTownUnitVar->moveTarget.unit; // This was null during a crash
						if (targetUnit == nullptr)
						{
							scbw::print("null unit in AI_progressBuild");
							return result;
						}
						s32 buildOrderId = (u8)targetUnit->mainOrderId;
						if (buildOrderId == OrderId::BuildTerran
						  || buildOrderId == OrderId::BuildProtoss1
						  || buildOrderId == OrderId::DroneLand
						  || buildOrderId == OrderId::DroneBuild
						  || buildOrderId == OrderId::DroneStartBuild)
						{
							u8* consoleVar = nullptr;
							s32 playerIdVar = (u8)playerId;
							u16 targetUnitQueue = targetUnit->buildQueue[(u8)targetUnit->buildQueueSlot % 5];
							s32 targetUnitQueue2 = targetUnitQueue;
							s32 targetQueueCostMinerals = units_dat::MineralCost[targetUnitQueue2];
							s32 targetQueueCostGas = units_dat::GasCost[targetUnitQueue2];
							AIMineralsReserve[playerIdVar] = targetQueueCostMinerals;
							AIGasReserve[playerIdVar] = targetQueueCostGas;
							if (helper::hasSuppliesForUnit(playerId, targetUnitQueue, 1))
							{
								if (gameMinerals[(u8)playerId] >= (s32)AIMineralsReserve[(u8)playerId])
								{
									if (gameGas[(u8)playerId] >= (s32)AIGasReserve[(u8)playerId])
									{
										*minerals -= AIMineralsReserve[playerId];
										*gas -= AIGasReserve[playerId];
									}
									else
									{
										consoleVar = consoleIndex;
										helper::displayErrorMessageIDWithSFX((u8)playerId, 851, *consoleVar + 150);
									}
								}
								else
								{
									consoleVar = consoleIndex;
									helper::displayErrorMessageIDWithSFX((u8)playerId, 850, *consoleVar + 147);
								}
							}
						}
						firstTownUnitVar = firstTownUnitVar->link.prev;
					}
					while (firstTownUnitVar);
				}
				result = firstTown->link.prev;
				firstTown = firstTown->link.prev;
			}
			while (firstTown);
		}
		return result;
	}

	// Hooked to change functionality later - Pr0nogo
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId) // 004A0F90
	{
		s16 regionId = helper::SAI_GetRegionIdFromPxEx(x, y);
		Unit* firstUnit = *firstVisibleUnit;
		s32 regionOffset = regionId << 6;
		s32 regionCenterX = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterX + regionOffset);
		s32 regionCenterY = *(s32*)((char*)&(*SAI_Paths)->regions[0].regionCenterY + regionOffset);

		Unit* result = nullptr;
		if (firstVisibleUnit)
		{
			// initialize vars used in loop
			Sprite* firstSprite;
			u32 fastDistance = 0;
			u32 maxDistance = 99999999;
			firstSprite = firstUnit->sprite;
			Unit* foundUnit = nullptr;
			u8 firstUnitPlayer;
			do
			{
				if (firstSprite)
				{
					if (firstUnit->mainOrderId)
					{
						if (firstUnit->status & UnitStatus::GroundedBuilding)
						{
							firstUnitPlayer = firstUnit->playerId;
							if (firstUnitPlayer == 11)
								firstUnitPlayer = firstSprite->playerId;
							if (!scbw::isAlliedTo(playerId, firstUnitPlayer))
							{
								fastDistance = helper::getDistanceFast(regionCenterX >> 8, firstUnit->sprite->position.x, regionCenterY >> 8, firstUnit->sprite->position.y);
								if (fastDistance < maxDistance)
								{
									maxDistance = fastDistance;
									foundUnit = firstUnit;
								}
							}
						}
					}
				}
				firstUnit = firstUnit->link.next;
			} while (firstUnit);
			result = foundUnit;
		}
		return result;
	}

	// We all know what this is for - Pr0nogo
	// TODO: Finish and clean up
	s32 AIManageBuildingPlacement(Unit* builder, u32 unitId, Point16 tryPosition, Point16* outPosition, s32 areaTiles) // 004465C0 - 3 nops
	{
		s32 playerVar;
		Unit* builderVar;
		char v7;
		s16 v8;
		s32 v9;
		s32 v10;
		s16 v12;
		s32 v13;
		Unit* disabledUnitVar;
		Sprite* disabledUnitSprite;
		s16 _x;
		s16 _y;
		s32 v18;
		s64 v19;
		u32 bestDistance;
		s32 posy;
		s32 v22;
		s32 posx;
		u32 currentDistance;
		bool v25;
		s16 v26;
		bool v27;
		s32 v28;
		s8 a1[4096];
		s32 v30;
		s32 v31;
		s32 v32;
		POINT _tileX;
		u32 bestFallbackDistance;
		s16* v35;
		s32* v36;
		s32 ownrId;
		s8* v38;
		Point16 __tileX;
		s16 v40;
		s32 v41;
		Point16 unitTypea;
		Point16 desiredFallbackPosition;

		_tileX.x = tryPosition.x / 32;
		__tileX.x = tryPosition.x / 32;
		builderVar = builder;
		playerVar = (u8)builder->playerId;
		_tileX.y = (*(s32*)&tryPosition >> 16) / 32;
		__tileX.y = _tileX.y;
		v7 = units_dat::BaseProperty[(u16)unitId];
		ownrId = playerVar;
		v40 = 0;
		v41 = 0;
		if (v7 & 2)
		{
			v9 = 4 * (u16)builderVar->id;
			v10 = *(s16*)((char*)&units_dat::BuildingDimensions[0].x + v9);
			*(Point16*)&v40 = __tileX;
			v9 = *(s16*)((char*)&units_dat::BuildingDimensions[0].y + v9) / -2 / 32 + __tileX.y;
			v36 = &units_dat::MaxHitPoints[(u16)unitId + 122];
			v8 = *(s16*)v36 / 32 + __tileX.x - v10 / 2 / 32;
			v35 = (s16*)((char*)&units_dat::MaxHitPoints[(u16)unitId + 122] + 2);
			v41 = *v35 / 32 + v9;
			v40 = v8;
			if (!helper::canBuildOnTileLocation(builderVar, ownrId, *(s32*)&v40, v41, unitId, 0, 0, 1, 0))
			{
				v12 = *(s16*)v36 / -32 + v8;
				v41 = *v35 / -32 + v41;
				v13 = units_dat::BuildingDimensions[(u16)builderVar->id].x;
				v40 = v12;
				outPosition->x = 32 * v12 + v13 / 2;
				outPosition->y = 32 * v41 + units_dat::BuildingDimensions[(u16)builderVar->id].y / 2;
				return 1;
			}
			playerVar = ownrId;
		}
		ownrId = 2500;
		v38 = (s8*)1;
		switch ((u16)unitId)
		{
			// Town halls
			case UnitId::TerranCommandCenter:
			case UnitId::ZergHatchery:
			case UnitId::ProtossNexus:
			{
				goto LABEL_10;
			}
			// Addons
			case UnitId::TerranComsatStation:
			case UnitId::TerranNuclearSilo:
			case UnitId::TerranQuarry:
			{
				unitId = UnitId::TerranCommandCenter;
			} break;
			case UnitId::TerranControlTower:
			case UnitId::TerranMunitionsBay:
			{
				unitId = UnitId::TerranStarport;
			} break;
			case UnitId::TerranMedbay:
			case UnitId::TerranCovertOps:
			{
				unitId = UnitId::TerranBarracks;
			} break;
			case UnitId::TerranMachineShop:
			case UnitId::TerranSalvoYard:
			{
				unitId = UnitId::TerranFactory;
			} break;
			case UnitId::TerranGuildhall:
			{
				unitId = UnitId::TerranCaptaincy;
			} break;
			case UnitId::TerranFutureStation:
			{
				unitId = UnitId::TerranScrapyard;
			} break;
			// Misc special rules
			case UnitId::TerranBunker:
			{
				goto LABEL_9;
			}
			case UnitId::ZergCreepColony:
			{
//				if (AIScriptController[playerVar].AI_Flags >= 0) // used SLOBYTE, probably checking "creep" asc3 command
				{
LABEL_9:
					if (helper::getBunkerPlacementPosition(playerVar, &ownrId))
					{
LABEL_10:
						v38 = 0;
					}
				}
			} break;
			case UnitId::ProtossPylon:
			{
				disabledUnitVar = helper::getDisabledUnitInRange(playerVar, tryPosition);
				disabledUnitArray[9] = disabledUnitVar;
				if (disabledUnitVar)
				{
					disabledUnitSprite = disabledUnitVar->sprite;
					_x = disabledUnitSprite->position.x;
					_y = disabledUnitSprite->position.y;
					_tileX.x = _x / 32;
					__tileX.x = _x / 32;
					tryPosition.x = _x;
					tryPosition.y = _y;
					_tileX.y = _y / 32;
					__tileX.y = _y / 32;
					v38 = 0;
				}				
			} break;
			default:
			{
			} break;
		}
		v18 = (u8)builderVar->playerId;
		*AIBuilderUnknown = builderVar;
		v32 = (u16)unitId;
		helper::AIGetBuildLocation((u16)unitId, (char*)a1, v18, tryPosition, areaTiles);
		*AIBuilderUnknown = 0;
		if (v38)
		{
			helper::AIPlacementDataSomething(a1, tryPosition);
		}
		unitTypea = __tileX;
		if (ownrId != 2500)
		{
			v19 = (s16)((*SAI_Paths)->regions[(u16)ownrId].regionCenterX >> 8);
//			LOWORD(unitTypea) = (((v19) & 0x1F) + (s32)v19) >> 5;
//			HIWORD(unitTypea) = (s16)((*SAI_Paths)->regions[(u16)ownrId].regionCenterY >> 8) / 32;
		}
		v30 = __tileX.y - 32;
		bestDistance = 99999999;
		bestFallbackDistance = 99999999;
		desiredFallbackPosition.x = 0;
		desiredFallbackPosition.y = 0;
		v36 = 0;
		v31 = __tileX.x - 32;
		while (1)
		{
			posy = v30;
			v38 = a1;
			v35 = (s16*)64;
			do
			{
				posx = v31;
				v22 = 0;
				__tileX.x = 0;
				__tileX.y = 0;
				do
				{
					if (v38[v22])
					{
//						currentDistance = helper::getTileDistance(32 * (s16)unitTypea, 32 * SHIWORD(unitTypea), 32 * posx, 32 * posy);
						if (currentDistance < bestDistance)
						{
							if (currentDistance <= areaTiles)
							{
								bestDistance = currentDistance;
								v40 = posx;
								v41 = posy;
							}
							else if (currentDistance < bestFallbackDistance)
							{
								bestFallbackDistance = currentDistance;
								desiredFallbackPosition.x = posx;
								desiredFallbackPosition.y = posy;
							}
						}
					}
					v22 = *(s32*)&__tileX + 1;
					++posx;
					++* (s32*)&__tileX;
				} while (*(u32*)&__tileX < 0x40);
				++posy;
				v25 = v35 == (s16*)1;
				v38 += 64;
				v35 = (s16*)((char*)v35 - 1);
			} while (!v25);

			if (bestDistance != 99999999)
			{
				break;
			}
			bestDistance = bestFallbackDistance;
			v26 = desiredFallbackPosition.x;
			*(Point16*)&v40 = desiredFallbackPosition;
			if (bestFallbackDistance != 99999999)
			{
				goto LABEL_36;
			}
			if (ownrId != 2500)
			{
				v27 = (u32)v36 + 1 < 2;
//				LOWORD(unitTypea) = _tileX.x;
//				HIWORD(unitTypea) = _tileX.y;
				v36 = (s32*)((char*)v36 + 1);
				if (v27)
				{
					continue;
				}
			}
			return 0;
		}
		v26 = v40;
LABEL_36:
		v28 = v32;
		outPosition->x = 32 * v26 + units_dat::BuildingDimensions[v32].x / 2;
		outPosition->y = 32 * v41 + units_dat::BuildingDimensions[v28].y / 2;
		return 1;
	}

	s16 getRegionIdFromUnit(Unit* unit) // 0049CB40
	{
		if (unit == nullptr || unit->sprite == nullptr)
		{
			return 0;
		}
		return helper::SAI_GetRegionIdFromPxEx(unit->sprite->position.x, unit->sprite->position.y);
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

	bool getBestSCVForRepairProc(Unit* repairman, Unit* repairTarget) // 004402B0
	{
		bool result = false;
		WorkerAi* repairmanAI = (WorkerAi*)repairman->pAI;
		if (repairman->playerId == repairTarget->playerId
			&& units_dat::BaseProperty[repairman->id] & UnitProperty::Worker
			&& !(repairman->status & UnitStatus::InTransport)
			&& !(repairman->status & UnitStatus::InBuilding) // new
			&& repairmanAI != nullptr
			&& repairmanAI->town->building_scv != nullptr) // new
		{
			result = repairmanAI->town->building_scv == repairman;
		}
		return result;
	}

	// Hooked to add sprite nullcheck - Pr0nogo
	b32 AI_MilitaryShouldWaitForGroup(Unit* unit) // 0043AA10
	{
		// new start
		if (unit == nullptr)
		{
			return false;
		}
		if (unit->sprite == nullptr)
		{
			return false;
		}
		// new end

		MilitaryAi* unitAI = (MilitaryAi*)unit->pAI;
		if (unitAI == nullptr)
		{
			return false;
		}
		if (unitAI->type != AiType::Military)
		{
			return false;
		}
		if (unitAI->parent != unit)
		{
			return false;
		}
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
		{
			return false;
		}
		if (unitAI->region->state == RegionState::Attacking)
		{
			return false;
		}

		AiTown* i = nullptr;
		AiTown* town = unitAI->region->town;
		for (i = town; town; i = town)
		{
			Unit* firstUnit = town->firstTownUnit;
			if (firstUnit != nullptr && firstUnit->sprite != nullptr && firstUnit != unit)
			{
				u32 distance = helper::getDistanceFast(firstUnit->sprite->position.x, unit->sprite->position.x, firstUnit->sprite->position.y, unit->sprite->position.y);
				if (distance <= 0x180 && helper::AI_AreOnConnectedRegions(firstUnit, unit) && !(helper::AI_CheckMilitaryGrouping(firstUnit)) && distance > 0x100)
				{
					return 1;
				}
				town = i;
			}
			town = town->link.prev;
		}

		// Not sure if this is correct
		if (town)
		{
			return true;
		}
		return false;
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
}