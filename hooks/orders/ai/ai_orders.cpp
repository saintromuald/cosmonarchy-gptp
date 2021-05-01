#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

#include "logger.h"
namespace hooks {
	void AI_TrainingUnitHook(CUnit* result, CUnit* parent) { // 004A2830
		bool isSuicideUnit = result->id == UnitId::ZergBroodling || result->id == UnitId::ZergBroodlisk || result->id == UnitId::ProtossSimulacrum;
		if (result == nullptr) return;
		if (parent == nullptr) return;
		if (result == parent && !isSuicideUnit && !(units_dat::BaseProperty[result->id] & UnitProperty::Worker)) return;
		if (playerTable[parent->playerId].type == PlayerType::Computer) {
			if (playerTable[result->playerId].type == PlayerType::Computer
				&& result->id == UnitId::TerranCleric) {
				if (!(AIScriptController[result->playerId].mainMedic))
					AIScriptController[result->playerId].mainMedic = result;

			}
			if (units_dat::BaseProperty[result->id] & UnitProperty::Worker) {
				helper::AI_TrainingWorker(result, parent);
			} else if (result->id == UnitId::ZergOvileth) {
				helper::AI_TrainingOverlord(result, parent); // replace with hook later
			} else if (isSuicideUnit) {
				AI_TrainingBroodling(parent, result);
			} else {
				helper::AI_TrainingNormal(result, parent);
			}
		}
	}

	/*
	//debug
	GPTP::logger << "Result id: " << result->id;
	if (parent && parent != result) {
		GPTP::logger << ", Parent id: " << parent->id;
	}
	GPTP::logger << '\n';

	if (playerTable[result->playerId].type == PlayerType::Computer) {
		if (playerTable[result->playerId].type == PlayerType::Computer
			&& result->id == UnitId::TerranMedic) {
			if (!(AIScriptController[result->playerId].mainMedic)) {
				AIScriptController[result->playerId].mainMedic = result;
			}
		}

		if (result->id == UnitId::ZergBroodling
		|| result->id == UnitId::ZergBroodlisk
//			|| result->id == UnitId::ProtossSimulacrum
			) {
				AI_TrainingBroodling(result, result);
				return;
			}

			if (parent != nullptr) {
				if (parent->pAI != nullptr
				||	result->status & UnitStatus::IsHallucination) {
					if (units_dat::BaseProperty[result->id] & UnitProperty::Worker) {
						AI_TrainingWorker(result, parent);
					}
					else if (result->id == UnitId::ZergOverlord) {
						AI_TrainingOverlord(result, parent); // replace with hook later
					}
					else {
						AI_TrainingNormal(result, parent);
					}
				}

			}
		}
	}*/

	/*	void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 alwaysThisRegion) {		// 0043DA20
		// alwaysThisRegion acts as a bool, not sure why IDA thinks it's int32
		// I assumed medicAI was UnitAI* but IDA tries to assign town values to it later
		// Looking at the aise version of this hook didn't help as it's more akin to a rewrite: https://pastebin.com/raw/8xNTUCuv
			AiCaptain* regionNew; // esi
			UnitAi* medicAI; // eax
			CUnit* medic; // edx
			UnitAi* nextAI; // ecx
			bool previousBool; // zf
			AiTown* AItown; // ecx

			regionNew = region;
			if (region->captainType == 3 && !alwaysThisRegion) {
				regionNew = &AiRegionCaptains[(uint8)unit->playerId][SAI_GetRegionIdFromPxEx(
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
						++* (int32*)&regionNew->unknown_0xA;
					unit->pAI = medicAI;
					AI_OrderToDestination(
						unit,
						unit->id != UnitId::TerranMedic ? 157 : 177,
						SAI_Paths->regions[(uint16)regionNew->region].rgnCenterX >> 8,
						SAI_Paths->regions[(uint16)regionNew->region].rgnCenterY >> 8);
					if (regionNew->captainType == 8 || regionNew->captainType == 9 || regionNew->captainType == 1 || regionNew->captainType == 2)
						assignCaptainToSlowestUnit(v3);
				}
			}
		}*/

		// Hooked to rule out AI crashes - Pr0nogo
	void AI_CreateNuke(CUnit* silo, CUnit* result) {								// 0045B7A0
		// originally had 3 args, but last one was unused
		CUnit* nuke; // eax
		CUnit* nuke2; // esi

		if (silo->id == UnitId::TerranNuclearSilo && !*(int32*)&silo->building.silo.isReady) {
			nuke = helper::createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (uint8)silo->playerId);
			nuke2 = nuke;
			if (nuke) {
				helper::updateUnitStatsFinishBuilding(nuke);
				helper::updateUnitStrengthAndApplyDefaultOrders(nuke2);
				helper::hideUnit(nuke2);
				silo->building.silo.nuke = nuke2;
				silo->building.silo.isReady = 1;
				nuke2->connectedUnit = silo;
				AI_TrainingUnitHook(nuke2, silo);
			}
		}
	}

	// Hooked to add region null check - Pr0nogo
	void AI_TrainingBroodling(CUnit* parent, CUnit* result) {						// 0043E280
		AiCaptain* region; // esi
		int16 regionType; // ax
		AiCaptain* playerRegion; // ecx
		int32 regionId; // esi
		int pX, pY;
		if (result->sprite != nullptr) {
			pX = result->sprite->position.x;
			pY = result->sprite->position.y;
		} else {
			pX = result->position.x;
			pY = result->position.y;
			GPTP::logger << "null sprite of Create Broodling detected!" << '\n';
		}
		regionId = helper::SAI_GetRegionIdFromPxEx(pX, pY);
		playerRegion = AiRegionCaptains[parent->playerId];
		if (playerRegion == nullptr) {
			GPTP::logger << "null region in AI_TrainingBroodling!" << '\n';
			return;
		}
		regionType = playerRegion[regionId].captainType;//state
		region = &playerRegion[regionId];
		if (region == nullptr) {
			GPTP::logger << "AI Broodling Bug: Null region!" << '\n';
		} else {
			if (regionType != 0) //region has assigned state, state 0 is nothing
				helper::AI_AssignMilitary(1, region);
			helper::AI_AddMilitary(region, result, 1);
		}
	}

	// Hooked to investigate crashes - Pr0nogo
/*	void AI_TrainingNormal(CUnit* a1, CUnit* a2) {									// 00435DB0
		int16 v2; // dx
		CUnit* v3; // esi
		int16 regionId; // ax
		UnitAi* v5; // ebx
		UnitAi* v6; // eax
		int32 v7; // edi
		AiTown* v8; // ebx
		AiCaptain* v9; // eax
		int16 v10; // dx
		int16 v11; // cx
		UnitAi* v12; // [esp+4h] [ebp-4h]

		v3 = a1;
		v2 = a1->id;
		if (v2 != UnitId::ZergGuardian && v2 != UnitId::ZergDevourer) {
			if (a1->status & 0x40000000) {
				regionId = SAI_GetRegionIdFromPxEx(a1->sprite->position.x, a1->sprite->position.y);
				AI_AddMilitary(&AiRegionCaptains[a1->playerId][regionId], a1, 1);
				return;
			}
			if (v2 != UnitId::ProtossArchon
				&& v2 != UnitId::ProtossMindTyrant
				&& v2 != UnitId::ZergLakizilisk
				&& v2 != UnitId::TerranNuclearMissile
				&& v2 != UnitId::ProtossWitness)
			{
				v5 = a2->pAI;
				v12 = a2->pAI;
				if (v5) {
					if (a2 == a1) {
						v7 = 0;
						AI_AssignWorker(a1);
						v6 = v3->pAI;
						if (v6)
						{
							v8 = v6->town;
							AI_TownInfo(v6, (UnitAi*)&v8->field_10);
							if (v8)
							{
								if (v3 == v8->field_28)
									v8->field_28 = 0;
								if (v3 == v8->field_24)
									v8->field_24 = 0;
							}
							v5 = v12;
							v3->pAI = 0;
						}
					}
					else if (a2->id == v2) {
						v7 = 0;
					}
					else {
						v7 = (uint8)a2->buildQueueSlot;
					}
					v9 = (AiCaptain*)*(&v5->field_18 + v7);
					if (*(&v5->field_9 + v7) == 2)
					{
						if (!*(int32*)&v9->field_C)
						{
							v10 = v9->fullAirStrength;
							v11 = v9->field_18;
							*(int32*)&v9->field_C = v3;
							v9->regionAirStrength = v10;
							v9->fullGndStrength = v11;
							v3->pAI = (UnitAi*)v9;
							return;
						}
						*(&v5->field_9 + v7) = 1;
						v9 = getAIRegionInfoFromUnitLocation(v3);
						*(&v5->field_18 + v7) = (int)v9;
					}
					if (*(&v5->field_9 + v7) == 1)
						AI_AddMilitary(v9, v3, 0);
					else
						AI_AddGuard(v3);
				}
			}
		}
	}*/

	// Hooked because iquare told me to - Pr0nogo
	char AI_AddUnitAi(CUnit* unit, AiTown* town) { // 00433DD0
		UnitAi* unitAI;
		WorkerAi* unitAIWorker;
		BuildingAi* unitAIBuilding;
		WorkerAiArray* workerAIArray;
		UnitAi* previous1;
		uint32* buildingQueueValues;
		uint8* buildingQueueTypes; // was char*
		int8 playerRACE;
		int32 playerID;
		uint16 unitType;

		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
			workerAIArray = town->free_workers;
			unitAI = (UnitAi*)(workerAIArray->first_free_worker);
			if (unitAI != nullptr) {
				previous1 = unitAI->link.prev;
				auto prevIsNull = unitAI->link.prev == 0;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (!prevIsNull)
					previous1->link.next = 0;
				if (unitAI != nullptr) {
					unitAI->link.next = 0;
					unitAI->link.prev = (UnitAi*)town->workers;
					if (town->workers) {
						town->workers->link.next = (WorkerAi*)unitAI;
					}
					town->workers = (WorkerAi*)unitAI;
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
		} else if ((units_dat::BaseProperty[unit->id] & 1 && unit->id != UnitId::ResourceVespeneGeyser)
			|| unit->id == UnitId::ZergLarva
			|| unit->id == UnitId::ZergEgg
			|| unit->id == UnitId::ZergOvileth) {
			unitAI = helper::AI_CreateBuildingAi(town->free_buildings);
			if (unitAI) {
				unitAI->type = 3; // building ai
				unitAIBuilding = (BuildingAi*)unitAI;
				unitAIBuilding->parent = unit;
				unitAIBuilding->town = town;
				buildingQueueValues = (uint32*)unitAIBuilding->trainQueueValues;
				for (int i = 0; i < 5; i++) {
					buildingQueueValues[i] = 0;
				}
				buildingQueueTypes = &unitAIBuilding->trainQueueTypes[0];
				for (int i = 0; i < 5; i++) {
					buildingQueueTypes[i] = 0;
				}
				unit->pAI = unitAI;
				if (unit->id == UnitId::ZergHatchery
					|| unit->id == UnitId::ZergLair
					|| unit->id == UnitId::ZergHive
					|| unit->id == UnitId::ZergSire
					) {
					if (unit->status & UnitStatus::Completed) {
						helper::orderComputerClear(unit, OrderId::ComputerAI);
						helper::setSecondaryOrder(unit, OrderId::SpreadCreep);
					}
				}
				if (!(AIScriptController[(uint8)town->player].AI_Flags.isUseMapSettings)
					|| unit->status & UnitStatus::GroundedBuilding
					&& unit->id != UnitId::TerranWatchdog
					&& unit->id != UnitId::TerranBunker
					&& unit->id != UnitId::ZergCreepColony
					&& unit->id != UnitId::ZergSporeColony
					&& unit->id != UnitId::ZergSunkenColony
					&& unit->id != UnitId::ZergLarvalColony
					&& unit->id != UnitId::ProtossPylon
					&& unit->id != UnitId::ProtossPhotonCannon) {
					helper::AI_MakeGuard(town->player, unit);
				}
				if (!town->inited) {
					town->inited = 1;
					town->mineral = 0;
					for (int i = 0; i < 3; i++) {
						town->gas_buildings[i] = 0;
					}
				}
				playerID = unit->playerId;
				playerRACE = playerTable[playerID].race;
				if (playerRACE == RaceId::Terran) {
					unitType = UnitId::TerranCommandCenter;
				} else if (playerRACE == RaceId::Protoss) {
					unitType = UnitId::ProtossNexus;
				} else {
					unitType = UnitId::ZergHatchery;
				}
				if (unit->id == unitType || helper::unitIsActiveResourceDepot(unit)) {
					unitAI = (UnitAi*)town->main_building->pAI;
					if (!unitAI) {
						town->main_building = unit;
					}
				} else {
					auto gas_structure_id = helper::getPlayerDefaultRefineryUnitType(playerID);
					if (unit->id == gas_structure_id || unit->id == UnitId::ResourceVespeneGeyser) {
						if (town->gas_buildings[0]) {
							if (town->gas_buildings[1]) {
								if (!town->gas_buildings[2]) {
									town->gas_buildings[2] = unit;
								}
							} else {
								town->gas_buildings[1] = unit;
							}
						} else {
							town->gas_buildings[0] = unit;
						}
						if (town->resource_area) {
							town->resource_area_not_set = 1;
						}
					}
				}
			}
		}
		return (char)unit->id;
	}

	void AI_BuildTechUpgrade(AiTown* town, int32 method, int32 amount, int32 id, int32 priority, int32 need) { // 00431F90
		int32 itr = 0; // eax
		constexpr int BuildCountFlag = 0xF8; //0xf8 are upper flags of 0xff, 0x1/0x2/0x4 are used by other flags
		TownReq* townUnits; // edx
		if (playerTable[(uint8)town->player].type == PlayerType::Computer) {
			townUnits = town->town_units;
			bool setNeed = false;
			if (town->town_units[0].flags_and_count & BuildCountFlag) //if build
			{
				for (int i = 0; i < 100; i++) {
					++townUnits;
					if (!(townUnits->flags_and_count & BuildCountFlag)) { //if not build
						setNeed = true;
						itr = i;
						break;
					}
				}
			} else {
				setNeed = true;
			}
			if (setNeed) {
				town->town_units[itr].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->town_units[itr].id = id;
				town->town_units[itr].priority = priority;
				// build orders have 0xf8 flag, tech has 0x4, upgrade has 0x2, 0x1 - only if needed
			}
		}
	}

	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
	void AI_UnitWasHit(CUnit* self, CUnit* attacker, int32 mainTargetReactions, int32 dontCallHelp) { // 0043F320
		BuildingAi* selfBuildingAi; // eax
		int32 quartTotalHP; // edi
		CUnit* target; // eax
		int32 v11; // eax
		UnitAi* v12; // esi
		int32 v13; // eax
		AiCaptain* v14; // esi
		int32 v15; // edx
		int32 v16; // eax
		AiCaptain* selfRegion; // edi
		char v18; // al
		int16 v19; // ax
		int16 v20; // di
		int32 v21; // esi
		int32 v22; // eax
		int32 v23; // edi
		int32 v24; // eax
		int32 v25; // esi
		int32 v26; // esi
		CUnit* v27; // esi
		uint16 v29; // ax
		__int64 v33; // rax
		CUnit* v34; // esi
		int32 v35; // eax
		CUnit* v36; // esi
		CUnit* v37; // eax
		int32 v38; // eax
		int32 v39; // [esp+Ch] [ebp-14h]
		int32 v40; // [esp+Ch] [ebp-14h]
		int32 v41; // [esp+14h] [ebp-Ch]
		int32 v42; // [esp+14h] [ebp-Ch]
		CUnit* realTarget; // [esp+1Ch] [ebp-4h]
		int32 v44; // [esp+2Ch] [ebp+Ch]
		MilitaryAi* militaryAi;

		if (playerTable[self->playerId].type != PlayerType::Computer || self->playerId >= 8u) {
			return;
		}
		if (units_dat::BaseProperty[(uint16)self->id] & UnitProperty::Building) {
			if (self->pAI) {
				AIScriptController[self->playerId].lastIndividualUpdateTime = *elapsedTimeSeconds;
				selfBuildingAi = (BuildingAi*)self->pAI;
				if (selfBuildingAi)
					selfBuildingAi->town->in_battle = 1;
				if (mainTargetReactions) {
					if (attacker) {
						if (units_dat::BaseProperty[(uint16)self->id] & UnitProperty::Worker // cancel structure code
							&& !(self->mainOrderId) // Original function call checks if unit has a sprite??? 00475A50
							&& !(self->status & UnitStatus::Completed)) {
							quartTotalHP = self->getTotalHitPointsPlusShields() / 4;
							if (self->getCurrentLifeInGame() < quartTotalHP) {
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
		if (target->status & UnitStatus::InBuilding) {
			realTarget = target->connectedUnit;
			if (!realTarget)
				return;
		}
		if (realTarget->status & UnitStatus::CloakingForFree) {
			if (!(realTarget->status & UnitStatus::CloakingForFree)) {
				realTarget = helper::checkArbiterCloakingRange(self, UnitId::ProtossDidact);
				if (!realTarget) {
					return;
				}
			}
		}
		if (mainTargetReactions) {
			helper::AI_OrderUnitCloaking(self);
			helper::AI_OrderLurkerBurrow(self);
		}
		v12 = (UnitAi*)self->pAI;
		v39 = 0;
		v41 = 0;
		if (!v12)
			goto LABEL_37;
		if (v12->type == 4) {
			militaryAi = (MilitaryAi*)v12;
			if (militaryAi->region->captainType == 1) {
				if (self->isUnitAINonBuilding())
					helper::AI_AssignMilitary(2, militaryAi->region);
			}
			v41 = helper::AI_IsUnderAttack(self, 0);
		}
		if (!mainTargetReactions || !helper::unitIsActiveTransport(self) || !(self->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (helper::isUnitLoaded(self)) {
			if (self->mainOrderId != 111)
				helper::orderComputerClear(self, 0x6Fu);
			return;
		}
		if (helper::AI_TryReturnHome(self, 0) || !helper::AI_TransportAction(self)) {
			if (AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings)
				v39 = 1;
		LABEL_37:
			v14 = &AiRegionCaptains[(uint8)self->playerId][helper::SAI_GetRegionIdFromPxEx(
				self->sprite->position.x,
				self->sprite->position.y)];
			if (!v39) {
				if (!dontCallHelp)
					helper::AI_AskForHelp(self, realTarget, v41);
				if (!v41) {
					if (!self->pAI
						|| helper::AI_IsUnderAttack(self, 0)
						|| (v42 = 1, *(int8*)(v15 + 8) == 1)
						&& AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings) {
						v42 = 0;
					}
					selfRegion = &AiRegionCaptains[(uint8)self->playerId][helper::getRegionIdFromUnit(realTarget)];
					if (v42) {
						v18 = v14->captainType;
						if (v18 == 5 || v18 == 4)
							helper::AI_AssignMilitary(6, v14);
						if (v14->captainType) {
							if (!selfRegion->captainType && *elapsedTimeSeconds > 60)
								helper::AI_AssignMilitary(5, selfRegion);
						}
					}
					if (!dontCallHelp) {
						selfRegion->enemyAirStrength = helper::getEnemyAirStrengthInRegion(selfRegion->region, (uint8)self->playerId);
						selfRegion->enemyGroundStrength = helper::getEnemyStrengthInRegion(selfRegion->region, (uint8)self->playerId, 0);
						v19 = selfRegion->neededAirStrength;
						if (v19 > v14->neededAirStrength)
							v14->neededAirStrength = v19;
						v20 = selfRegion->neededGroundStrength;
						if (v20 > v14->neededGroundStrength)
							v14->neededGroundStrength = v20;
					}
				}
			}
			v14->captainFlags |= 0x20u;
			if (self->status & UnitStatus::Completed) {
				v21 = (uint8)self->mainOrderId;
				v40 = (uint8)self->mainOrderId;
				if (v21 != OrderId::CompletingArchonSummon && v21 != OrderId::ResetCollision1 && v21 != OrderId::ConstructingBuilding) {
					if (mainTargetReactions || !scbw::orderIsSpell(v21)) {
						v23 = (uint16)self->id;
						if (self->id || v21 != OrderId::EnterTransport) {
							v44 = helper::unitIsPerformingUnbreakableCode(self);
							if (v44 || !helper::AI_CastSpellBehavior(self, mainTargetReactions)) {
								if (v21 != OrderId::RechargeShields1 && v21 != OrderId::Move) {
									if (!mainTargetReactions)
										goto LABEL_126;
									if (v23 != UnitId::ProtossSolarion
										//										|| (uint8)(LOBYTE(self->field_C8) + BYTE1(self->carrier.inHangarCount)) > 2u
										|| !helper::AI_TransportAction(self)) {
										if (realTarget && realTarget->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings
											|| ((v25 = 3 * self->getTotalHitPointsPlusShields() / 4, self->getCurrentLifeInGame() > v25)
												|| v23 != UnitId::ProtossDidact && v23 != UnitId::TerranWraith && v23 != UnitId::ZergMutalisk
												|| !helper::AI_TransportAction(self))
											&& ((v26 = 2 * self->getTotalHitPointsPlusShields() / 3, self->getCurrentLifeInGame() > v26)
												|| v23 != UnitId::TerranMinotaur && v23 != UnitId::ProtossSolarion && v23 != UnitId::ProtossPanoptus
												|| !helper::AI_TransportAction(self))) {
											if (self->id != UnitId::ZergLakizilisk
												|| self->status & UnitStatus::Burrowed
												|| v40 == OrderId::Burrow
												|| v40 == OrderId::Unburrow) {
												v27 = realTarget;
											} else {
												v27 = realTarget;
												if (helper::AI_Flee(self, realTarget))
													return;
											}
											if (v44
												|| !(self->status & UnitStatus::IsBuilding)
												|| helper::unitHasGroundPath(self, v27->sprite->position.x, v27->sprite->position.y)
												|| !helper::AI_Flee(self, v27)) {
											LABEL_126:
												if (self->orderTarget.unit != realTarget) { // Responding with workers
													if (!(units_dat::BaseProperty[(uint16)self->id] & UnitProperty::Worker)
														|| (v29 = realTarget->id, units_dat::BaseProperty[v29] & UnitProperty::Worker)
														|| v29 == UnitId::ZergZergling
														|| helper::AI_HasNoDefenseBuild((uint8)self->id)) {
														if (mainTargetReactions) {
															if (self->pAI) {
																if (helper::unitCanSeeCloakedTarget(realTarget, self))
																	helper::AI_RespondToCloakedThreat(self, realTarget);
															}
														}
														if (!v44) {
															if (helper::unitCanAttackTarget(realTarget, self, 1)) {
																v36 = self->subunit;
																v37 = self->subunit;
																self->autoTargetUnit = realTarget;
																if (v37->isSubunit())
																	v36->autoTargetUnit = realTarget;
																helper::AI_UpdateAttackTarget(self, 0, 1, 0);
															} else if (self->pAI) {
																if (v40 != 159
																	//																	|| (v33 = AI_PrepareToFlee(realTarget, self), (int32)v33 == *(int32*)&self->sprite->position)
																	) {
																	if (mainTargetReactions) {
																		if (self->id != 34) {
																			v34 = self->orderTarget.unit;
																			if (!v34 || !helper::unitCanAttackTarget(v34, v34, 1))
																				helper::AI_Flee(self, realTarget);
																		}
																	}
																} else {
																	//																	orderInterrupt(self, 6u, (Point32*)v33, (CUnit*)HIDWORD(v33)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													} else if (mainTargetReactions) {
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
	void AI_IterateBuildingAI(CUnit* result, CUnit* parent) { // 00435770
		UnitAi* parentAI; // eax
		AiTown* parentTown; // esi
		BuildingAi* removeBuilding; // eax
		AiTown* removeBuildingTown; // edi
		CUnit* workerUnit; // esi
		CUnit* workerTarget; // ecx
		int32 queueCount; // eax
		int32 queueCount2;
		BuildingAi* j; // esi
		CUnit* builderUnit; // ecx
		uint32 i; // [esp+8h] [ebp-4h]
		WorkerAi* worker;
		BuildingAi* building;

		if (playerTable[(uint8)parent->playerId].type == PlayerType::Computer) {
			parentAI = (UnitAi*)parent->pAI;
			if (parentAI) {
				if (parentAI->type == 2) {
					worker = (WorkerAi*)parentAI;
					parentTown = worker->town;
				} else { // assumed BuildingAi, unlikely to be GuardAi, can't be anything else
					building = (BuildingAi*)parentAI;
					parentTown = building->town;
				}
				if (parent == result) {
					helper::AI_RemoveFromTowns(result);
					removeBuilding = (BuildingAi*)result->pAI;
					if (removeBuilding) {
						removeBuildingTown = removeBuilding->town;
						helper::AI_DeleteBuildingAI(removeBuilding, removeBuildingTown->free_buildings);
						if (removeBuildingTown) {
							if (result == removeBuildingTown->building_scv) {
								removeBuildingTown->building_scv = 0;
							}
							if (result == removeBuildingTown->main_building) {
								removeBuildingTown->main_building = 0;
							}
						}
						result->pAI = 0;
					}
				}
				AI_AddUnitAi(result, parentTown);
				if (result->id != UnitId::TerranBunker || *elapsedTimeSeconds <= 60) {
					if (result->id == UnitId::TerranWatchdog) {
						workerUnit = parentTown->workers->parent;
						for (i = 0; workerUnit; i += queueCount) {
							workerTarget = workerUnit->moveTarget.unit;
							if (workerTarget->id == UnitId::TerranWatchdog) {
								queueCount = 1;
							} else {
								queueCount = helper::AI_GetQueuedCount(workerTarget, UnitId::TerranWatchdog);
							}
							//workerUnit = *(uint32*)workerUnit; // not sure if this is even necessary, does v8 need to be iterated upon every loop?
						}
						for (j = parentTown->buildings; j; i += queueCount2) {
							builderUnit = j->parent;
							if (builderUnit->id == UnitId::TerranWatchdog) {
								queueCount2 = 1;
							} else {
								queueCount2 = helper::AI_GetQueuedCount(builderUnit, UnitId::TerranWatchdog);
							}
							j = j->link.prev;
						}
						if (i >= 4) {
							helper::AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 1, result->sprite->position.x);
						}
					}
				} else {
					helper::AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 5, result->sprite->position.x);
					helper::AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 1, result->sprite->position.x);
				}
			}
		}
	}

	// hooked for attack extender
//	0x0043E670 => train_attack_force(@eax *mut AiRegion)->uint32;
	uint32 AI_TrainAttackForce(AiCaptain* region) {					// 0043E670
		uint32 result; // eax
		uint32 v2; // edi
		AiCaptain* v3; // esi
		uint16 v4; // bx
		uint32 v5; // edi
		uint32 v6; // ebx
		uint32 v7; // edi
		uint32 v8; // ebx
		uint32 v9[63]; // [esp+Ch] [ebp-108h]
		uint32 v10; // [esp+108h] [ebp-Ch]
		uint32 v11; // [esp+10Ch] [ebp-8h]
		uint32 v12; // [esp+110h] [ebp-4h]

		v3 = region;
		v2 = (uint8)region->playerId;
		result = AI_GetMissingAttackForceUnits(region, v9, AIScriptController[v2].AI_Flags.isUseMapSettings);
		v10 = result;
		if (AIScriptController[v2].AI_Flags.isUseMapSettings
			|| (v5 = AIScriptController[v2].lastIndividualUpdateTime) != 0
			&& (v12 = *elapsedTimeSeconds - v5 >= 30, *elapsedTimeSeconds - v5 < 30)) {
			v11 = 60;
		} else {
			v11 = 160;
		}
		v6 = 0;
		v12 = 0;
		if (result) {
			do {
				v7 = v9[v6];
				if (v7 < UnitId::None && !helper::AI_AddRegionToMilitary(v3, v7, v11)) {
					v8 = (uint8)v3->playerId;
					AIScriptController[v8].AI_Flags.flag_0x40 = 1; // Might be wrong
					helper::AI_FindUnit(v7, v8, 1);
					helper::AI_AddSpendingRequest(v7, v8, 1, 60, v3);
					helper::AI_AddBaseUnitSpendingRequest(v7, v8, v3, 60);
					v6 = v12;
				}
				result = v10;
				v12 = ++v6;
			} while (v6 < v10);
		}
		return result;
	}

	//	0x0043EEC0 => grouping_for_attack(*mut AiRegion);
	void AI_GroupingForAttack(AiCaptain* region) { // 0043EEC0
		AiCaptain* regionVar; // esi
		CUnit* attackTarget; // eax
		AiCaptain* targetRegion; // esi
		uint32 missingUnits; // eax
		uint32 attackUnit; // ecx
		uint16 regionId; // ax
		AiCaptain* targetRegion2; // esi
		AiCaptain* targetRegion3; // edi
		uint32 attackWave[63]; // [esp+8h] [ebp-108h]
		int32 a3; // [esp+104h] [ebp-Ch]
		int32 a4; // [esp+108h] [ebp-8h]
		int32 a2; // [esp+10Ch] [ebp-4h]

		helper::AI_RecalculateRegionStrength(region);
		helper::AI_TargetEnemyInRegion(region);
		if (region->captainFlags & 0x20 && helper::AI_MoveMilitaryToDefend(region)) {
			return;
		}
		helper::AI_GetRegionMilitaryTypes(region, &a2, &a3, &a4);
		regionVar = region;
		if (a2 && (*SAI_Paths)->regions[region->region].accessabilityFlags == 0x1FFD || helper::AI_CanTravelToRegion(region, region)) {
			helper::AI_AssignMilitary(0, region);
			attackTarget = helper::AI_PickAttackTarget_NoBuildings(region);
			if (attackTarget) {
				targetRegion = &AiRegionCaptains[(uint8)region->playerId][helper::SAI_GetRegionIdFromPxEx(
					attackTarget->sprite->position.x,
					attackTarget->sprite->position.y)];
				helper::AI_AssignMilitary(9, targetRegion);
				helper::AI_MoveMilitaryToOtherRegion(targetRegion, region, 0, 0, 1);
			} else {
				helper::AI_IterateTownSomething(region);
			}
			return;
		}
		if (helper::AI_AttackTimerDecrement((uint8)region->playerId)) {
		LABEL_15:
			regionId = region->targetRegion;
			if (regionId) {
				targetRegion2 = &AiRegionCaptains[(uint8)region->playerId][regionId];
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
		if (!missingUnits) {
		LABEL_14:
			if (helper::AI_HasForcesMovingToAttack((uint8)region->playerId))
				return;
			goto LABEL_15;
		}
		while ((uint32)attackWave[attackUnit] >= 0xE4) {
			if (++attackUnit >= missingUnits)
				goto LABEL_14;
		}
	}

	//	0x00438050 => get_missing_attack_force_units_hook(*mut AiRegion,*mut uint32,bool)->uint32;
	uint32 AI_GetMissingAttackForceUnits(AiCaptain* region, uint32* unitIds, uint32 isCampaign) { // 00438050
		int16 v3; // dx
		uint16* v4; // ecx
		uint32 i; // edi
		uint32 v6; // eax
		CUnit* v7; // esi
		int8 v8; // bl
		uint32 v9; // ecx
		uint32 v10; // edx // was unitIDs, not sure what it should be here
		int32 v11; // eax
		uint32 v12; // esi
		int32 v13; // ecx
		UnitAi* v14; // ecx
		uint32 v15; // esi
		uint32 v16; // ecx
		CUnit* v18; // [esp+8h] [ebp-4h]
		MilitaryAi* military;

		v4 = AIScriptController[(uint8)region->playerId].attackers;
		v3 = *v4;
		for (i = 0; *v4; ++i) {
			unitIds[i] = v3 - 1;
			++v4;
			v3 = *v4;
		}
		v6 = (uint8)region->playerId;
		v7 = firstPlayerUnit->unit[v6];
		v18 = firstPlayerUnit->unit[v6];
		if (v7) {
			while (1) {
				if (v7->sprite) {
					v8 = v7->mainOrderId;
					if (v8) {
						v9 = (uint16)v7->id;
						if (!(units_dat::BaseProperty[v9] & UnitProperty::Subunit))
							break;
					}
				}
			LABEL_37:
				v18 = v7->player_link.next;
				if (!v18)
					return i;
				v7 = v7->player_link.next;
			}
			v10 = (uint16)v7->id;
			if (v9 == UnitId::TerranPhalanxSiegeMode)
				v10 = UnitId::TerranPhalanxTankMode;
			v11 = 1;
			v12 = v7->status & 1;
			if (!v12 || v8 == OrderId::CompletingArchonSummon || v8 == OrderId::ResetCollision1) {
				if (isCampaign) {
				LABEL_36:
					v7 = v18;
					goto LABEL_37;
				}
				if (v10 == UnitId::ZergEgg || v10 == UnitId::ZergCocoon) {
					v10 = (uint16)v18->buildQueue[(uint8)v18->buildQueueSlot];
					v13 = units_dat::BaseProperty[v10];
					if (v13 & 0x400)
						v11 = 2;
				}
			}
			if (!(units_dat::ComputerAIInternal[v10] & 1)) {
				v14 = (UnitAi*)v18->pAI;
				military = (MilitaryAi*)v18->pAI;
				if (!v12
					|| v8 == 0x6A // UNI_FIRST_BLDG
					|| v8 == UnitId::Special_MatureChrysalis
					|| v14 != 0 && v14->type == 4 && military->region == region) {
					if (v11) {
						v15 = i - 1;
						do {
							v16 = 0;
							if (i) {
								while (v10 != unitIds[v16]) {
									if (++v16 >= i)
										goto LABEL_35;
								}
								unitIds[v16] = UnitId::None;
								if (v16 == v15) {
									--i;
									--v15;
								}
							}
						LABEL_35:
							--v11;
						} while (v11);
					}
				}
			}
			goto LABEL_36;
		}
		return i;
	}

	//	0x00447230 => ai_add_to_attackforce(@edi uint32,@eax uint32,@ecx uint32);
	void AI_AddToAttack(uint32 result, uint32 player, uint32 unitId) { // 00447230
		uint32 counter; // esi
		uint32 index; // eax
		uint32 plOffset; // edx
		uint16* atkGroups; // ecx
		uint16* atkFinal; // ecx

		counter = result;
		if (player < 8 && result < 63 && unitId < UnitId::None) {
			plOffset = 628 * player;
			index = 0;
			if (AIScriptController[player].attackers[0]) {
				atkGroups = AIScriptController[player].attackers;
				while (index < 63) {
					++atkGroups;
					++index;
					if (!*atkGroups)
						goto LABEL_8;
				}
			} else {
			LABEL_8:
				if (counter) {
					atkFinal = &AIScriptController[0].attackers[plOffset + index];
					do {
						--counter;
						if (index >= 63)
							break;
						*atkFinal = unitId + 1;
						++index;
						++atkFinal;
					} while (counter);
				}
			}
		}
	}

	//	0x0043ABB0 => ai_attack_to_hook(uint32,i32,i32,bool,bool)->uint32;
	bool AI_AttackManager(uint32 player, int32 x1, int32 y1, uint32 alywaysOverwriteLastAttack, uint32 allowFallbackAirOnly) { // 0043ABB0
		uint16 attackWave; // ax
		uint16* attackWave2; // ecx
		char regionsList[2500]; // [esp+4h] [ebp-9C8h]
		uint32 waveBool; // [esp+9C8h] [ebp-4h]

		if (AIScriptController[player].supplyCollection) {
			return 1;
		}
		if (AIScriptController[player].AI_AttackGroup) {
			return *((uint8*)&AiRegionCaptains[player][(uint16)AIScriptController[player].AI_AttackGroup] - 47) != 8;
		}
		memset(regionsList, 0, sizeof(regionsList));
		helper::PopulateRegionsWithOwn(regionsList, player);
		helper::PopulateRegionsWithEnemy(player, regionsList);
		while (helper::AI_GetNeighborRegionTypes(regionsList)) {
			helper::AI_FinishRegionAccessibility(regionsList);
		}
		attackWave = AIScriptController[player].attackers[0];
		attackWave2 = AIScriptController[player].attackers;
		waveBool = attackWave != 0;
		if (attackWave) {
			while (units_dat::DestroyScore[2 * attackWave + 226] & 4) {
				attackWave = attackWave2[1];
				++attackWave2;
				if (!attackWave)
					goto LABEL_12;
			}
			waveBool = 0;
		}
	LABEL_12:
		AIScriptController[player].AI_Flags.isSecureFinished = 1;
		if (!helper::AI_SetAttackGroupingRegion(waveBool, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
			&& !helper::AI_SetAttackGroupingRegion(waveBool, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack)) {
			if (!allowFallbackAirOnly || waveBool) {
				return 1;
			}
			if (!helper::AI_SetAttackGroupingRegion(1, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
				&& !helper::AI_SetAttackGroupingRegion(1, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack)) {
				AIScriptController[player].supplyCollection = 1;
				return 1;
			}
		}
		return 0;
	}

	//	0x00437F20 => has_units_for_attack_force_hook(@edx uint32)->uint32;
	int32 AI_HasUnitsForAttackForce(uint32 player) { // 00437F20
		uint16* v2; // eax
		uint16 v3; // cx
		uint32 i; // edi
		CUnit* v5; // edx
		uint32 v6; // eax
		uint8 v7; // bl
		uint32 v8; // ecx
		int32 v9; // esi
		uint32 v10; // ecx
		uint32 v11; // ecx
		uint32 v13[63]; // [esp+4h] [ebp-FCh]

		v2 = AIScriptController[player].attackers;
		v3 = *v2;
		for (i = 0; *v2; ++i) {
			v13[i] = v3 - 1;
			++v2;
			v3 = *v2;
		}
		v5 = firstPlayerUnit->unit[player];
		if (v5) {
			v6 = i - 1;
			do {
				if (v5->sprite) {
					v7 = v5->mainOrderId;
					if (v7) {
						v8 = (uint16)v5->id;
						if (!(units_dat::BaseProperty[v8] & 0x10)) {
							v9 = (uint16)v5->id;
							if (v8 == 30)
								v9 = 5;
							if (!(units_dat::ComputerAIInternal[v9] & 1)) {
								if (v5->status & 1) {
									if (v7 != 106 && v7 != 150 && !orders_dat::UseWeaponTargeting[v7]) {
										if (v5->pAI) {
											v10 = 0;
											if (i) {
												while (v9 != v13[v10]) {
													if (++v10 >= i)
														goto LABEL_22;
												}
												v13[v10] = 228;
												if (v10 == v6) {
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
				v5 = v5->player_link.next;
			} while (v5);
		}
		v11 = 0;
		if (!i)
			return 1;
		while (v13[v11] == 228) {
			if (++v11 >= i)
				return 1;
		}
		return 0;
	}
}