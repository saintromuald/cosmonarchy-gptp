#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace {
	// for AI_TrainingUnit
	char AI_TrainingWorker(CUnit* result, CUnit* parent);											// 00435700
	void AI_TrainingOverlord(CUnit* result, CUnit* parent);											// 00435770
	void AI_TrainingNormal(CUnit* result, CUnit* parent);											// 00435DB0

	// for AI_AddMilitary
	int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y);														// 0049C9F0
	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y);								// 0043D5D0
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region);											// 00436F70

	// for AI_CreateNuke
	CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId);									// 004A09D0
	void updateUnitStatsFinishBuilding(CUnit* unit);												// 004A01F0
	uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);										// 0049FA40
	void hideUnit(CUnit* unit);																		// 004E6340

	// for AI_TrainingBroodling
	void AI_AssignMilitary(int32 regionType, AiCaptain* region);										// 004390A0
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 always_this_region);					// 0043DA20

	// for AI_AddUnitAi
	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);										// 004044C0
	void orderComputerClear(CUnit* unit, uint8 orderId);												// 00475310
	CUnit* setSecondaryOrder(CUnit* result, char orderId);											// 004743D0
	void AI_MakeGuard(int32 playerId, CUnit* unit);													// 0043A010
	bool unitIsActiveResourceDepot(CUnit* unit);													// 00401CF0
	int32 getPlayerDefaultRefineryUnitType(int32 playerId);												// 004320D0

	// for AI_UnitWasHit // some of these should probably be methods in api/etc
	char AI_CancelStructure(CUnit* structure);														// 00468280
	CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target);										// 0043AD60
	CUnit* checkArbiterCloakingRange(CUnit* unit, uint16 id);											// 004410C0
	void AI_OrderUnitCloaking(CUnit* unit);															// 0043B970
	void AI_OrderLurkerBurrow(CUnit* unit);															// 0043C230
	bool AI_IsUnderAttack(CUnit* unit, int32 flag);													// 00436E70
	bool unitIsActiveTransport(CUnit* unit);														// 004E6BA0
	int32 isUnitLoaded(CUnit* unit);																	// 004E7110
	int32 AI_TryReturnHome(CUnit* unit, int32 dontIssueOrder);											// 00462EA0
	int32 AI_TransportAction(CUnit* unit);															// 0043DB50
	int32 AI_AskForHelp(CUnit* self, CUnit* attacker, int32 onlyNearbyGuards);							// 0043ADA0
	int16 getRegionIdFromUnit(CUnit* unit);															// 0049CB40
	int32 getEnemyAirStrengthInRegion(uint16 regionId, int32 player);										// 00431D00
	int32 getEnemyStrengthInRegion(uint16 regionId, int32 player, int32 includeAir);							// 004318E0
	bool unitIsPerformingUnbreakableCode(CUnit* unit);												// 00402310
	bool AI_CastSpellBehavior(CUnit* unit, bool isAggro);											// 004A13C0
	int32 AI_Flee(CUnit* unit, CUnit* enemy);															// 0043E400
	bool unitHasGroundPath(CUnit* unit, int16 x, int16 y);												// 0042FA00
	bool AI_HasNoDefenseBuild(int32 player);															// 00447370
	bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit);										// 00401D60
	void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit);										// 0043C580
	bool unitCanAttackTarget(CUnit* target, CUnit* unit, int32 checkDetection);						// 00476730
	int32 AI_UpdateAttackTarget(CUnit* unit, int32 acceptIfSieged, int32 acceptCritters, int32 mustReach);	// 00477160
//	__int64 AI_PrepareToFlee(CUnit* unit, CUnit* enemy);											// 00476A50
	void orderInterrupt(CUnit* unit, uint8 orderId, Point32* pos, CUnit* target);						// 00474C30

	// for AI_IterateBuildingAI
	uint16 AI_RemoveFromTowns(CUnit* worker);															// 00432760
	BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList);			// 00404500
	int32 AI_GetQueuedCount(CUnit* builder, uint32 unitId);												// 00466B70
	void AI_PrepareGuardAI(uint16 y, uint32 player, uint16 unitId, uint16 x);									// 00462670

	// Attack extender

	// for AI_TrainAttackForce
	int32 AI_AddRegionToMilitary(AiCaptain* region, int32 unitId, int32 priority);						// 0043E2E0
	uint32 AI_FindUnit(uint32 unitId, uint32 playerId, uint32 type);											// 00447DC0
	uint32 AI_AddSpendingRequest(uint16 unitId, uint32 type, uint8 player, char priority, void* value);			// 00447980
	uint32 AI_AddBaseUnitSpendingRequest(uint32 unitId, uint32 player, AiCaptain* region, char priority);	// 004484A0

	// for AIRegion_GroupForAttack
	uint32 AI_RecalculateRegionStrength(AiCaptain* region);											// 0043A390
	CUnit* AI_TargetEnemyInRegion(AiCaptain* region);												// 0043CC40
	int32 AI_MoveMilitaryToDefend(AiCaptain* region);													// 0043DE40
	void AI_GetRegionMilitaryTypes(AiCaptain* region, int32* hasGroundMilitary,						// 00438FC0
		int32* cantAttackGround, int32* cantAttackAir);
	int32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2);								// 0043A790
	CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region);										// 0043A510
	uint32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, uint32 pullUnitsNearRegion,	// 0043E580
		uint32 sendPureSpellcasters, uint32 dontSendFromBunkers);
	AiTown* AI_IterateTownSomething(AiCaptain* region);												// 0043DD20
	uint32 AI_AttackTimerDecrement(uint32 player);														// 00447090
	int32 AI_HasForcesMovingToAttack(uint32 player);														// 0043A2E0

	// for AI_AttackManager
	uint16 PopulateRegionsWithOwn(char* regionData, uint32 player);										// 00438B30
	void PopulateRegionsWithEnemy(uint32 player, char* regionData);									// 00439B50
	int32 AI_GetNeighborRegionTypes(char* regionData);												// 00436D80
	void AI_FinishRegionAccessibility(char* regionData);											// 00436CF0
	int32 AI_SetAttackGroupingRegion(uint32 airAttack, uint32 alsoVeryNear, uint32 player, char* regionData,	// 004399D0
		int32 x, int32 y, uint32 alwaysOverwriteLastAttack);

}
#include "logger.h"
namespace hooks {
	void AI_TrainingUnit(CUnit* result, CUnit* parent) { // 004A2830
		bool isSuicideUnit = result->id == UnitId::ZergBroodling || result->id == UnitId::ZergBroodlisk || result->id == UnitId::ProtossSimulacrum;
		if (result == nullptr) return;
		if (parent == nullptr) return;
		if (result == parent && !isSuicideUnit) return;
		if (playerTable[parent->playerId].type == PlayerType::Computer) {
			if (playerTable[result->playerId].type == PlayerType::Computer
				&& result->id == UnitId::TerranCleric) {
				if (!(AIScriptController[result->playerId].mainMedic))
					AIScriptController[result->playerId].mainMedic = result;

			}
			if (units_dat::BaseProperty[result->id] & UnitProperty::Worker) {
				AI_TrainingWorker(result, parent);
			} else if (result->id == UnitId::ZergOvileth) {
				AI_TrainingOverlord(result, parent); // replace with hook later
			} else if (isSuicideUnit) {
				AI_TrainingBroodling(parent, result);
			} else {
				AI_TrainingNormal(result, parent);
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
			nuke = createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (uint8)silo->playerId);
			nuke2 = nuke;
			if (nuke) {
				updateUnitStatsFinishBuilding(nuke);
				updateUnitStrengthAndApplyDefaultOrders(nuke2);
				hideUnit(nuke2);
				silo->building.silo.nuke = nuke2;
				silo->building.silo.isReady = 1;
				nuke2->connectedUnit = silo;
				AI_TrainingUnit(nuke2, silo);
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
		regionId = SAI_GetRegionIdFromPxEx(pX, pY);
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
				AI_AssignMilitary(1, region);
			AI_AddMilitary(region, result, 1);
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
			unitAI = AI_CreateBuildingAi(town->free_buildings);
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
						orderComputerClear(unit, OrderId::ComputerAI);
						setSecondaryOrder(unit, OrderId::SpreadCreep);
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
					AI_MakeGuard(town->player, unit);
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
				if (unit->id == unitType || unitIsActiveResourceDepot(unit)) {
					unitAI = (UnitAi*)town->main_building->pAI;
					if (!unitAI) {
						town->main_building = unit;
					}
				} else {
					auto gas_structure_id = getPlayerDefaultRefineryUnitType(playerID);
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
								AI_CancelStructure(self); // originally unused variable v8
								return;
							}
						}
					}
				}
			}
		}
		target = AI_GetActualTarget(self, attacker);
		realTarget = target;
		if (target->status & UnitStatus::InBuilding) {
			realTarget = target->connectedUnit;
			if (!realTarget)
				return;
		}
		if (realTarget->status & UnitStatus::CloakingForFree) {
			if (!(realTarget->status & UnitStatus::CloakingForFree)) {
				realTarget = checkArbiterCloakingRange(self, UnitId::ProtossDidact);
				if (!realTarget) {
					return;
				}
			}
		}
		if (mainTargetReactions) {
			AI_OrderUnitCloaking(self);
			AI_OrderLurkerBurrow(self);
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
					AI_AssignMilitary(2, militaryAi->region);
			}
			v41 = AI_IsUnderAttack(self, 0);
		}
		if (!mainTargetReactions || !unitIsActiveTransport(self) || !(self->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (isUnitLoaded(self)) {
			if (self->mainOrderId != 111)
				orderComputerClear(self, 0x6Fu);
			return;
		}
		if (AI_TryReturnHome(self, 0) || !AI_TransportAction(self)) {
			if (AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings)
				v39 = 1;
		LABEL_37:
			v14 = &AiRegionCaptains[(uint8)self->playerId][SAI_GetRegionIdFromPxEx(
				self->sprite->position.x,
				self->sprite->position.y)];
			if (!v39) {
				if (!dontCallHelp)
					AI_AskForHelp(self, realTarget, v41);
				if (!v41) {
					if (!self->pAI
						|| AI_IsUnderAttack(self, 0)
						|| (v42 = 1, *(int8*)(v15 + 8) == 1)
						&& AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings) {
						v42 = 0;
					}
					selfRegion = &AiRegionCaptains[(uint8)self->playerId][getRegionIdFromUnit(realTarget)];
					if (v42) {
						v18 = v14->captainType;
						if (v18 == 5 || v18 == 4)
							AI_AssignMilitary(6, v14);
						if (v14->captainType) {
							if (!selfRegion->captainType && *elapsedTimeSeconds > 60)
								AI_AssignMilitary(5, selfRegion);
						}
					}
					if (!dontCallHelp) {
						selfRegion->enemyAirStrength = getEnemyAirStrengthInRegion(selfRegion->region, (uint8)self->playerId);
						selfRegion->enemyGroundStrength = getEnemyStrengthInRegion(selfRegion->region, (uint8)self->playerId, 0);
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
							v44 = unitIsPerformingUnbreakableCode(self);
							if (v44 || !AI_CastSpellBehavior(self, mainTargetReactions)) {
								if (v21 != OrderId::RechargeShields1 && v21 != OrderId::Move) {
									if (!mainTargetReactions)
										goto LABEL_126;
									if (v23 != UnitId::ProtossSolarion
										//										|| (uint8)(LOBYTE(self->field_C8) + BYTE1(self->carrier.inHangarCount)) > 2u
										|| !AI_TransportAction(self)) {
										if (realTarget && realTarget->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(uint8)self->playerId].AI_Flags.isUseMapSettings
											|| ((v25 = 3 * self->getTotalHitPointsPlusShields() / 4, self->getCurrentLifeInGame() > v25)
												|| v23 != UnitId::ProtossDidact && v23 != UnitId::TerranWraith && v23 != UnitId::ZergMutalisk
												|| !AI_TransportAction(self))
											&& ((v26 = 2 * self->getTotalHitPointsPlusShields() / 3, self->getCurrentLifeInGame() > v26)
												|| v23 != UnitId::TerranMinotaur && v23 != UnitId::ProtossSolarion && v23 != UnitId::ProtossPanoptus
												|| !AI_TransportAction(self))) {
											if (self->id != UnitId::ZergLakizilisk
												|| self->status & UnitStatus::Burrowed
												|| v40 == OrderId::Burrow
												|| v40 == OrderId::Unburrow) {
												v27 = realTarget;
											} else {
												v27 = realTarget;
												if (AI_Flee(self, realTarget))
													return;
											}
											if (v44
												|| !(self->status & UnitStatus::IsBuilding)
												|| unitHasGroundPath(self, v27->sprite->position.x, v27->sprite->position.y)
												|| !AI_Flee(self, v27)) {
											LABEL_126:
												if (self->orderTarget.unit != realTarget) { // Responding with workers
													if (!(units_dat::BaseProperty[(uint16)self->id] & UnitProperty::Worker)
														|| (v29 = realTarget->id, units_dat::BaseProperty[v29] & UnitProperty::Worker)
														|| v29 == UnitId::ZergZergling
														|| AI_HasNoDefenseBuild((uint8)self->id)) {
														if (mainTargetReactions) {
															if (self->pAI) {
																if (unitCanSeeCloakedTarget(realTarget, self))
																	AI_RespondToCloakedThreat(self, realTarget);
															}
														}
														if (!v44) {
															if (unitCanAttackTarget(realTarget, self, 1)) {
																v36 = self->subunit;
																v37 = self->subunit;
																self->autoTargetUnit = realTarget;
																if (v37->isSubunit())
																	v36->autoTargetUnit = realTarget;
																AI_UpdateAttackTarget(self, 0, 1, 0);
															} else if (self->pAI) {
																if (v40 != 159
																	//																	|| (v33 = AI_PrepareToFlee(realTarget, self), (int32)v33 == *(int32*)&self->sprite->position)
																	) {
																	if (mainTargetReactions) {
																		if (self->id != 34) {
																			v34 = self->orderTarget.unit;
																			if (!v34 || !unitCanAttackTarget(v34, v34, 1))
																				AI_Flee(self, realTarget);
																		}
																	}
																} else {
																	//																	orderInterrupt(self, 6u, (Point32*)v33, (CUnit*)HIDWORD(v33)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													} else if (mainTargetReactions) {
														AI_Flee(self, realTarget);
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
					AI_RemoveFromTowns(result);
					removeBuilding = (BuildingAi*)result->pAI;
					if (removeBuilding) {
						removeBuildingTown = removeBuilding->town;
						AI_DeleteBuildingAI(removeBuilding, removeBuildingTown->free_buildings);
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
								queueCount = AI_GetQueuedCount(workerTarget, UnitId::TerranWatchdog);
							}
							//workerUnit = *(uint32*)workerUnit; // not sure if this is even necessary, does v8 need to be iterated upon every loop?
						}
						for (j = parentTown->buildings; j; i += queueCount2) {
							builderUnit = j->parent;
							if (builderUnit->id == UnitId::TerranWatchdog) {
								queueCount2 = 1;
							} else {
								queueCount2 = AI_GetQueuedCount(builderUnit, UnitId::TerranWatchdog);
							}
							j = j->link.prev;
						}
						if (i >= 4) {
							AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 1, result->sprite->position.x);
						}
					}
				} else {
					AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 5, result->sprite->position.x);
					AI_PrepareGuardAI(result->sprite->position.y, (uint8)result->playerId, 1, result->sprite->position.x);
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
				if (v7 < UnitId::None && !AI_AddRegionToMilitary(v3, v7, v11)) {
					v8 = (uint8)v3->playerId;
					AIScriptController[v8].AI_Flags.flag_0x40 = 1; // Might be wrong
					AI_FindUnit(v7, v8, 1);
					AI_AddSpendingRequest(v7, v8, 1, 60, v3);
					AI_AddBaseUnitSpendingRequest(v7, v8, v3, 60);
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

		AI_RecalculateRegionStrength(region);
		AI_TargetEnemyInRegion(region);
		if (region->captainFlags & 0x20 && AI_MoveMilitaryToDefend(region)) {
			return;
		}
		AI_GetRegionMilitaryTypes(region, &a2, &a3, &a4);
		regionVar = region;
		if (a2 && (*SAI_Paths)->regions[region->region].accessabilityFlags == 0x1FFD || AI_CanTravelToRegion(region, region)) {
			AI_AssignMilitary(0, region);
			attackTarget = AI_PickAttackTarget_NoBuildings(region);
			if (attackTarget) {
				targetRegion = &AiRegionCaptains[(uint8)region->playerId][SAI_GetRegionIdFromPxEx(
					attackTarget->sprite->position.x,
					attackTarget->sprite->position.y)];
				AI_AssignMilitary(9, targetRegion);
				AI_MoveMilitaryToOtherRegion(targetRegion, region, 0, 0, 1);
			} else {
				AI_IterateTownSomething(region);
			}
			return;
		}
		if (AI_AttackTimerDecrement((uint8)region->playerId)) {
		LABEL_15:
			regionId = region->targetRegion;
			if (regionId) {
				targetRegion2 = &AiRegionCaptains[(uint8)region->playerId][regionId];
				AI_AssignMilitary(1, targetRegion2);
				targetRegion3 = targetRegion2;
				regionVar = region;
				AI_MoveMilitaryToOtherRegion(targetRegion3, region, 0, 0, 1);
			}
			AI_AssignMilitary(0, regionVar);
			return;
		}
		missingUnits = AI_GetMissingAttackForceUnits(region, attackWave, 1);
		attackUnit = 0;
		if (!missingUnits) {
		LABEL_14:
			if (AI_HasForcesMovingToAttack((uint8)region->playerId))
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
		PopulateRegionsWithOwn(regionsList, player);
		PopulateRegionsWithEnemy(player, regionsList);
		while (AI_GetNeighborRegionTypes(regionsList)) {
			AI_FinishRegionAccessibility(regionsList);
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
		if (!AI_SetAttackGroupingRegion(waveBool, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
			&& !AI_SetAttackGroupingRegion(waveBool, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack)) {
			if (!allowFallbackAirOnly || waveBool) {
				return 1;
			}
			if (!AI_SetAttackGroupingRegion(1, 0, player, regionsList, x1, y1, alywaysOverwriteLastAttack)
				&& !AI_SetAttackGroupingRegion(1, 1, player, regionsList, x1, y1, alywaysOverwriteLastAttack)) {
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

namespace {
	// for AI_TrainUnit
	const uint32 Func_AI_TrainingWorker = 0x00435700;
	char AI_TrainingWorker(CUnit* result, CUnit* parent) {
		static int32 resultFunc;
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingWorker
			MOV resultFunc, EAX
			POPAD
		}
		return resultFunc;
	}

	const uint32 Func_AI_TrainingOverlord = 0x00435770;
	void AI_TrainingOverlord(CUnit* result, CUnit* parent) {
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingOverlord
			POPAD
		}
	}

	const uint32 Func_AI_TrainingNormal = 0x00435DB0;
	void AI_TrainingNormal(CUnit* result, CUnit* parent) {
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingNormal
			POPAD
		}
	}

	// For AI_AddMilitary

	const uint32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
	int16 SAI_GetRegionIdFromPxEx(int32 x, int32 y) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EDI, x
			MOV ECX, y
			CALL Func_SAI_GetRegionIdFromPxEx
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	const uint32 Func_AI_OrderToDestination = 0x0043D5D0;
	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y) {
		__asm {
			PUSHAD
			PUSH y
			PUSH x
			PUSH orderId
			PUSH unit
			CALL Func_AI_OrderToDestination
			POPAD
		}
	}

	const uint32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region) {
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_AssignCaptainToSlowestUnit
			POPAD
		}
	}

	// for AI_CreateNuke

	const uint32 Func_createUnit = 0x004A09D0;
	CUnit* createUnit(int16 unitType, int32 x, int32 y, int32 playerId) {
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH playerId
			PUSH y
			MOV EAX, x
			MOVZX CX, unitType
			CALL Func_createUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
	void updateUnitStatsFinishBuilding(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	const uint32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
	uint32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStrengthAndApplyDefaultOrders
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_hideUnit = 0x004E6340;
	void hideUnit(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hideUnit
			POPAD
		}
	}

	// for AI_TrainingBroodling

	const uint32 Func_AI_AssignMilitary = 0x004390A0;
	void AI_AssignMilitary(int32 regionType, AiCaptain* region) {
		__asm {
			PUSHAD
			MOV EBX, regionType
			MOV ESI, region
			CALL Func_AI_AssignMilitary
			POPAD
		}
	}

	const uint32 Func_AI_AddMilitary = 0x0043DA20;
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, int32 always_this_region) {
		__asm {
			PUSHAD
			PUSH always_this_region
			MOV EAX, region
			MOV EBX, unit
			CALL Func_AI_AddMilitary
			POPAD
		}
	}

	// for AI_AddUnitAi

	const uint32 Func_AI_CreateBuildingAi = 0x004044C0;
	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI) {
		static UnitAi* result;
		__asm {
			PUSHAD
			MOV ESI, buildingAI
			CALL Func_AI_CreateBuildingAi
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_orderComputerClear = 0x00475310;
	void orderComputerClear(CUnit* unit, uint8 orderId) {
		__asm {
			PUSHAD
			MOV ESI, unit
			MOVZX CL, orderId
			CALL Func_orderComputerClear
			POPAD
		}
	}

	const uint32 Func_setSecondaryOrder = 0x004743D0;
	CUnit* setSecondaryOrder(CUnit* result, char orderId) {
		static CUnit* result2;
		__asm {
			PUSHAD
			MOV EAX, result
			MOVZX CL, orderId
			CALL Func_setSecondaryOrder
			MOV result2, EAX
			POPAD
		}
		return result2;
	}

	const uint32 Func_AI_MakeGuard = 0x0043A010;
	void AI_MakeGuard(int32 playerId, CUnit* unit) {
		__asm {
			PUSHAD
			PUSH unit
			PUSH playerId
			CALL Func_AI_MakeGuard
			POPAD
		}
	}

	const uint32 Func_unitIsActiveResourceDepot = 0x00401CF0;
	bool unitIsActiveResourceDepot(CUnit* unit) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_unitIsActiveResourceDepot
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
	int32 getPlayerDefaultRefineryUnitType(int32 playerId) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, playerId
			CALL Func_getPlayerDefaultRefineryUnitType
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AI_UnitWasHit // some of these should probably be methods in api/etc
	const uint32 Func_AI_CancelStructure = 0x00468280;
	char AI_CancelStructure(CUnit* structure) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, structure
			CALL Func_AI_CancelStructure
			MOV result, EAX
			POPAD
		}
		return (char)result;
	}

	const uint32 Func_AI_GetActualTarget = 0x0043AD60;
	CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target) {
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EDI, attacker
			MOV ESI, target
			CALL Func_AI_GetActualTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_checkArbiterCloakingRange = 0x004410C0;
	CUnit* checkArbiterCloakingRange(CUnit* unit, uint16 id) {
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH id
			MOV EAX, unit
			CALL Func_checkArbiterCloakingRange
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_OrderUnitCloaking = 0x0043B970;
	void AI_OrderUnitCloaking(CUnit* unit) {
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_OrderUnitCloaking
			POPAD
		}
	}

	const uint32 Func_AI_OrderLurkerBurrow = 0x0043C230;
	void AI_OrderLurkerBurrow(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_AI_OrderLurkerBurrow
			POPAD
		}
	}

	const uint32 Func_AI_IsUnderAttack = 0x00436E70;
	bool AI_IsUnderAttack(CUnit* unit, int32 flag) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH flag
			MOV EAX, unit
			CALL Func_AI_IsUnderAttack
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitIsActiveTransport = 0x004E6BA0;
	bool unitIsActiveTransport(CUnit* unit) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, unit
			CALL Func_unitIsActiveTransport
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_isUnitLoaded = 0x004E7110;
	int32 isUnitLoaded(CUnit* unit) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_isUnitLoaded
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_TryReturnHome = 0x00462EA0;
	int32 AI_TryReturnHome(CUnit* unit, int32 dontIssueOrder) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH dontIssueOrder
			MOV EAX, unit
			CALL Func_AI_TryReturnHome
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_TransportAction = 0x0043DB50;
	int32 AI_TransportAction(CUnit* unit) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_AI_TransportAction
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AskForHelp = 0x0043ADA0;
	int32 AI_AskForHelp(CUnit* self, CUnit* attacker, int32 onlyNearbyGuards) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH onlyNearbyGuards
			PUSH attacker
			PUSH self
			CALL Func_AI_AskForHelp
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getRegionIdFromUnit = 0x0049CB40;
	int16 getRegionIdFromUnit(CUnit* unit) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_getRegionIdFromUnit
			MOV result, EAX
			POPAD
		}
		return (int16)result;
	}

	const uint32 Func_getEnemyAirStrengthInRegion = 0x00431D00;
	int32 getEnemyAirStrengthInRegion(uint16 regionId, int32 player) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH player
			MOVZX AX, regionId
			CALL Func_getEnemyAirStrengthInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_getEnemyStrengthInRegion = 0x004318E0;
	int32 getEnemyStrengthInRegion(uint16 regionId, int32 player, int32 includeAir) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH includeAir
			PUSH player
			MOVZX AX, regionId
			CALL Func_getEnemyStrengthInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_unitIsPerformingUnbreakableCode = 0x00402310;
	bool unitIsPerformingUnbreakableCode(CUnit* unit) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_unitIsPerformingUnbreakableCode
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_AI_CastSpellBehavior = 0x004A13C0;
	bool AI_CastSpellBehavior(CUnit* unit, bool isAggro) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH isAggro
			MOV EAX, unit
			CALL Func_AI_CastSpellBehavior
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_AI_Flee = 0x0043E400;
	int32 AI_Flee(CUnit* unit, CUnit* enemy) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH enemy
			MOV EAX, unit
			CALL Func_AI_Flee
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_unitHasGroundPath = 0x0042FA00;
	bool unitHasGroundPath(CUnit* unit, int16 x, int16 y) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH y
			MOVZX DX, x
			MOV EAX, unit
			CALL Func_unitHasGroundPath
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_AI_HasNoDefenseBuild = 0x00447370;
	bool AI_HasNoDefenseBuild(int32 player) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_AI_HasNoDefenseBuild
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_unitCanSeeCloakedTarget = 0x00401D60;
	bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, target
			CALL Func_unitCanSeeCloakedTarget
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_AI_RespondToCloakedThreat = 0x0043C580;
	void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit) {
		__asm {
			PUSHAD
			PUSH unit
			MOV EAX, threat
			CALL Func_AI_RespondToCloakedThreat
			POPAD
		}
	}

	const uint32 Func_unitCanAttackTarget = 0x00476730;
	bool unitCanAttackTarget(CUnit* target, CUnit* unit, int32 checkDetection) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH checkDetection
			MOV ESI, unit
			MOV EBX, target
			CALL Func_unitCanAttackTarget
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const uint32 Func_AI_UpdateAttackTarget = 0x00477160;
	int32 AI_UpdateAttackTarget(CUnit* unit, int32 acceptIfSieged, int32 acceptCritters, int32 mustReach) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH mustReach
			PUSH acceptCritters
			PUSH acceptIfSieged
			MOV ECX, unit
			CALL Func_AI_UpdateAttackTarget
			MOV result, EAX
			POPAD
		}
		return result;
	}

	/*	const uint32 Func_AI_PrepareToFlee = 0x00476A50;
		__int64 AI_PrepareToFlee(CUnit* enemy, CUnit* unit) {
			static __int64 result;
			__asm {
				PUSH unit
				MOV EAX, enemy
				CALL Func_AI_PrepareToFlee
				MOV result, EAX
				POPAD
			}
			return result;
		}*/

	const uint32 Func_orderInterrupt = 0x00474C30;
	void orderInterrupt(CUnit* unit, uint8 orderId, Point32* pos, CUnit* target) {
		__asm {
			PUSHAD
			PUSH target
			PUSH pos
			PUSH orderId
			MOV ESI, unit
			CALL Func_orderInterrupt
			POPAD
		}
	}

	// for AI_IterateBuildingAI
	const uint32 Func_AI_RemoveFromTowns = 0x00432760;
	uint16 AI_RemoveFromTowns(CUnit* worker) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, worker
			CALL Func_AI_RemoveFromTowns
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	const uint32 Func_AI_DeleteBuildingAI = 0x00404500;
	BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList) {
		static BuildingAi* result;
		__asm {
			PUSHAD
			MOV EAX, building
			MOV EDX, buildingList
			CALL Func_AI_DeleteBuildingAI
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_GetQueuedCount = 0x00466B70;
	int32 AI_GetQueuedCount(CUnit* builder, uint32 unitId) {
		static int32 result;
		__asm {
			PUSHAD
			MOV ECX, builder
			MOV EDI, unitId
			CALL Func_AI_GetQueuedCount
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_PrepareGuardAI = 0x00462670;
	void AI_PrepareGuardAI(uint16 y, uint32 player, uint16 unitId, uint16 x) {
		__asm {
			PUSHAD
			PUSH x
			PUSH unitId
			MOV ESI, player
			MOVZX DI, y
			CALL Func_AI_PrepareGuardAI
			POPAD
		}
	}

	// Attack extender

	// for AI_TrainAttackForce
	const uint32 Func_AI_AddRegionToMilitary = 0x0043E2E0;
	int32 AI_AddRegionToMilitary(AiCaptain* region, int32 unitId, int32 priority) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH priority
			PUSH unitId
			MOV EAX, region
			CALL Func_AI_AddRegionToMilitary
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_FindUnit = 0x00447DC0;
	uint32 AI_FindUnit(uint32 unitId, uint32 playerId, uint32 type) {
		static uint32 typeXD = type;
		static uint32 result;
		__asm {
			PUSHAD
			PUSH typeXD
			MOV EBX, playerId
			MOV EAX, unitId
			CALL Func_AI_FindUnit
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AddSpendingRequest = 0x00447980;
	uint32 AI_AddSpendingRequest(uint16 unitId, uint32 type, uint8 player, char priority, void* value) {
		static uint32 typeXD = type;
		static uint32 result;
		__asm {
			PUSHAD
			PUSH value
			PUSH priority
			MOVZX CL, player
			MOV EDX, typeXD
			MOVZX AX, unitId
			CALL Func_AI_AddSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AddBaseUnitSpendingRequest = 0x004484A0;
	uint32 AI_AddBaseUnitSpendingRequest(uint32 unitId, uint32 player, AiCaptain* region, char priority) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH priority
			MOV ESI, region
			MOV ECX, player
			MOV EAX, unitId
			CALL Func_AI_AddBaseUnitSpendingRequest
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AIRegion_GroupForAttack
	const uint32 Func_AI_RecalculateRegionStrength = 0x0043A390;
	uint32 AI_RecalculateRegionStrength(AiCaptain* region) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL Func_AI_RecalculateRegionStrength
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_TargetEnemyInRegion = 0x0043CC40;
	CUnit* AI_TargetEnemyInRegion(AiCaptain* region) {
		static CUnit* result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_TargetEnemyInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_MoveMilitaryToDefend = 0x0043DE40;
	int32 AI_MoveMilitaryToDefend(AiCaptain* region) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EDI, region
			CALL Func_AI_TargetEnemyInRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_GetRegionMilitaryTypes = 0x00438FC0;
	void AI_GetRegionMilitaryTypes(AiCaptain* region, int32* hasGroundMilitary,
		int32* cantAttackGround, int32* cantAttackAir) {
		__asm {
			PUSHAD
			MOV EAX, region
			MOV EBX, hasGroundMilitary
			MOV EDI, cantAttackGround
			MOV ESI, cantAttackAir
			CALL Func_AI_GetRegionMilitaryTypes
			POPAD
		}
	}

	const uint32 Func_AI_CanTravelToRegion = 0x0043A790;
	int32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH region2
			PUSH region1
			CALL Func_AI_CanTravelToRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_PickAttackTarget_NoBuildings = 0x0043A510;
	CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region) {
		static CUnit* result;
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_PickAttackTarget_NoBuildings
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_MoveMilitaryToOtherRegion = 0x0043E580;
	uint32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, uint32 pullUnitsNearRegion,
		uint32 sendPureSpellcasters, uint32 dontSendFromBunkers) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH dontSendFromBunkers
			PUSH sendPureSpellcasters
			PUSH pullUnitsNearRegion
			MOV ESI, source
			MOV EDI, target
			CALL Func_AI_MoveMilitaryToOtherRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_IterateTownSomething = 0x0043DD20;
	AiTown* AI_IterateTownSomething(AiCaptain* region) {
		static AiTown* result;
		__asm {
			PUSHAD
			MOV ESI, region
			CALL Func_AI_IterateTownSomething
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_AttackTimerDecrement = 0x00447090;
	uint32 AI_AttackTimerDecrement(uint32 player) {
		static uint32 result;
		__asm {
			PUSHAD
			MOV ECX, player
			CALL Func_AI_AttackTimerDecrement
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_HasForcesMovingToAttack = 0x0043A2E0;
	int32 AI_HasForcesMovingToAttack(uint32 player) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, player
			CALL Func_AI_HasForcesMovingToAttack
			MOV result, EAX
			POPAD
		}
		return result;
	}

	// for AI_AttackManager
	const uint32 Func_PopulateRegionsWithOwn = 0x00438B30;
	uint16 PopulateRegionsWithOwn(char* regionData, uint32 player) {
		static uint32 result;
		__asm {
			PUSHAD
			PUSH player
			MOV EBX, regionData
			CALL Func_PopulateRegionsWithOwn
			MOV result, EAX
			POPAD
		}
		return (uint16)result;
	}

	const uint32 Func_PopulateRegionsWithEnemy = 0x00439B50;
	void PopulateRegionsWithEnemy(uint32 player, char* regionData) {
		__asm {
			PUSHAD
			PUSH regionData
			PUSH player
			CALL Func_PopulateRegionsWithEnemy
			POPAD
		}
	}

	const uint32 Func_AI_GetNeighborRegionTypes = 0x00436D80;
	int32 AI_GetNeighborRegionTypes(char* regionData) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_GetNeighborRegionTypes
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_AI_FinishRegionAccessibility = 0x00436CF0;
	void AI_FinishRegionAccessibility(char* regionData) {
		__asm {
			PUSHAD
			MOV EBX, regionData
			CALL Func_AI_FinishRegionAccessibility
			POPAD
		}
	}

	const uint32 Func_AI_SetAttackGroupingRegion = 0x004399D0;
	int32 AI_SetAttackGroupingRegion(uint32 airAttack, uint32 alsoVeryNear, uint32 player, char* regionData,
		int32 x, int32 y, uint32 alwaysOverwriteLastAttack) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH alwaysOverwriteLastAttack
			PUSH y
			PUSH x
			PUSH regionData
			PUSH player
			MOV EDX, alsoVeryNear
			MOV ECX, airAttack
			CALL Func_AI_SetAttackGroupingRegion
			MOV result, EAX
			POPAD
		}
		return result;
	}
}