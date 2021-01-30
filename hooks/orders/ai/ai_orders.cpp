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
	s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);														// 0049C9F0
	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y);								// 0043D5D0
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region);											// 00436F70

	// for AI_CreateNuke
	CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId);									// 004A09D0
	void updateUnitStatsFinishBuilding(CUnit* unit);												// 004A01F0
	u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);										// 0049FA40
	void hideUnit(CUnit* unit);																		// 004E6340

	// for AI_TrainingBroodling
	void AI_AssignMilitary(s32 regionType, AiCaptain* region);										// 004390A0
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 always_this_region);					// 0043DA20

	// for AI_AddUnitAi
	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);										// 004044C0
	void orderComputerClear(CUnit* unit, u8 orderId);												// 00475310
	CUnit* setSecondaryOrder(CUnit* result, char orderId);											// 004743D0
	void AI_MakeGuard(s32 playerId, CUnit* unit);													// 0043A010
	bool unitIsActiveResourceDepot(CUnit* unit);													// 00401CF0
	s32 getPlayerDefaultRefineryUnitType(s32 playerId);												// 004320D0

	// for AI_UnitWasHit // haven't wrapped any of these yet, some should probably be methods in api etc
	char AI_CancelStructure(CUnit* structure, s32 a2);												// 00468280
	CUnit* AI_GetActualTarget(CUnit* attacker, CUnit* target);										// 0043AD60
	CUnit* checkArbiterCloakingRange(CUnit* unit, u16 id);											// 004410C0
	void AI_OrderUnitCloaking(CUnit* unit);															// 0043B970
	void AI_OrderLurkerBurrow(CUnit* unit);															// 0043C230
	bool AI_IsUnderAttack(CUnit* unit, s32 flag);													// 00436E70
	bool unitIsActiveTransport(CUnit* unit);														// 004E6BA0
	s32 isUnitLoaded(CUnit* unit);																	// 004E7110
	void orderComputerClear(CUnit* unit, u8 orderId);												// 00475310
	s32 AI_TryReturnHome(CUnit* unit, s32 a2);														// 00462EA0
	s32 AI_TransportAction(CUnit* unit);															// 0043DB50
	s32 AI_AskForHelp(CUnit* self, CUnit* attacker, s32 onlyNearbyGuards);							// 0043ADA0
	s16 getRegionIdFromUnit(CUnit* unit);															// 0049CB40
	s32 getEnemyAirStrengthInRegion(u16 regionId, s32 player);										// 00431D00
	s32 getEnemyStrengthInRegion(u16 regionId, s32 player, s32 includeAir);							// 004318E0
	bool unitIsPerformingUnbreakableCode(CUnit* unit);												// 00402310
	bool AI_CastSpellBehavior(CUnit* unit, bool isAggro);											// 004A13C0
	s32 AI_Flee(CUnit* unit, CUnit* enemy);															// 0043E400
	bool unitHasPathToDestOnGround(CUnit* unit, s16 x, s16 y);										// 0042FA00
	bool AI_HasNoDefenseBuild(s32 player);															// 00447370
	bool unitCanSeeCloakedTarget(CUnit* target, CUnit* unit);										// 00401D60
	void AI_RespondToCloakedThreat(CUnit* threat, CUnit* unit);										// 0043C580
	bool unitCanAttackTarget(CUnit* target, CUnit* unit, s32 checkDetection);						// 00476730
	s32 burrowIdle_Normal(CUnit* unit, s32 a2, s32 a3, s32 a4);										// 00477160
	__int64 AI_PrepareToFlee(CUnit* unit, CUnit* a2);												// 00476A50
	void orderInterrupt(CUnit* unit, u8 orderId, Point32* pos, CUnit* a4);							// 00474C30

	// for AI_IterateBuildingAI
	u16 AI_RemoveFromTowns(CUnit* worker);															// 00432760
	BuildingAi* AI_DeleteBuildingAI(BuildingAi* building, BuildingAiArray* buildingList);			// 00404500
	s32 AI_GetQueuedCount(CUnit* builder, u32 unitId);												// 00466B70
	void AI_PrepareGuardAI(u16 y, u32 player, u16 unitId, u16 x);									// 00462670

	// for trainAttackForce
	u32 getMissingAttackForceUnits(AiCaptain* region, s32* outUnitIds, s32 onlyAcceptCompleted);	// 00438050
	s32 AI_AddRegionToMilitary(AiCaptain* region, s32 unitId, s32 priority);						// 0043E2E0
	u32 AI_FindUnit(u32 unitId, u32 playerId, u32 type);											// 00447DC0
	u32 AI_AddSpendingRequest(u16 unitId, u32 type, u8 player, char priority, void* value);			// 00447980
	u32 AI_AddBaseUnitSpendingRequest(u32 unitId, u32 player, AiCaptain* region, char priority);	// 004484A0

	// for AIRegion_GroupForAttack
	u32 AI_RecalculateRegionStrength(AiCaptain* region);											// 0043A390
	CUnit* AI_TargetEnemyInRegion(AiCaptain* region);												// 0043CC40
	s32 AI_MoveMilitaryToDefend(AiCaptain* region);													// 0043DE40
	void AI_GetRegionMilitaryTypes(AiCaptain* region, s32* hasGroundMilitary,						// 00438FC0
		s32* cantAttackGround, s32* cantAttackAir);
	s32 AI_CanTravelToRegion(AiCaptain* region1, AiCaptain* region2);								// 0043A790
	CUnit* AI_PickAttackTarget_NoBuildings(AiCaptain* region);										// 0043A510
	u32 AI_MoveMilitaryToOtherRegion(AiCaptain* target, AiCaptain* source, u32 pullUnitsNearRegion,	// 0043E580
		u32 sendPureSpellcasters, u32 dontSendFromBunkers);
	AiTown* AI_IterateTownSomething(AiCaptain* region);												// 0043DD20
	u32 AI_AttackTimerDecrement(u32 player);														// 00447090
	u32 AI_GetMissingAttackForceUnits(AiCaptain* region, s32* outUnitIds, u32 isCampaign);			// 0043E580
	s32 AI_HasForcesMovingToAttack(u32 player);														// 0043A2E0

	// for AI_AttackManager
	u16 PopulateRegionsWithOwn(char* regionData, u32 player);										// 00438B30
	void PopulateRegionsWithEnemy(u32 player, char* regionData);									// 00439B50
	s32 AI_GetNeighborRegionTypes(char* regionData);												// 00436D80
	void Ai_FinishRegionAccessibility(char* regionData);											// 00436CF0
	s32 AI_SetAttackGroupingRegion(u32 airAttack, u32 alsoVeryNear, u32 player, char* regionData,	// 004399D0
		s32 x, s32 y, u32 alwaysOverwriteLastAttack);

}
#include "logger.h"
namespace hooks {
	void AI_TrainingUnit(CUnit* result, CUnit* parent) {							// 004A2830
		if (playerTable[parent->playerId].type == PlayerType::Computer) {
			if (playerTable[result->playerId].type == PlayerType::Computer
				&& result->id == UnitId::TerranMedic) {
				if (!(AIScriptController[result->playerId].mainMedic))
					AIScriptController[result->playerId].mainMedic = result;

			}
			if (units_dat::BaseProperty[result->id] & UnitProperty::Worker) {
				AI_TrainingWorker(result, parent);
			}
			else if (result->id == UnitId::ZergOverlord) {
				AI_TrainingOverlord(result, parent); // replace with hook later
			}
			else if (result->id == UnitId::ZergBroodling) {
				AI_TrainingBroodling(parent, result);
			}
			else {
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
	
/*	void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 alwaysThisRegion) {		// 0043DA20
	// alwaysThisRegion acts as a bool, not sure why IDA thinks it's s32
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

		if (silo->id == UnitId::TerranNuclearSilo && !*(s32*)&silo->building.silo.isReady) {
			nuke = createUnit(UnitId::TerranNuclearMissile, silo->sprite->position.x, silo->sprite->position.y, (u8)silo->playerId);
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
		s16 regionType; // ax
		AiCaptain* playerRegion; // ecx
		s32 regionId; // esi
		int pX, pY;
		if (result->sprite != nullptr) {
			pX = result->sprite->position.x;
			pY = result->sprite->position.y;
		}
		else {
			pX = result->position.x;
			pY = result->position.y;
			GPTP::logger << "NULL sprite of Create Broodling detected!" << '\n';
		}
		regionId = SAI_GetRegionIdFromPxEx(pX,pY);
		playerRegion = AiRegionCaptains[parent->playerId];
		if (playerRegion == NULL) {
			GPTP::logger << "NULL region in AI_TrainingBroodling!" << '\n';
			return;
		}
		regionType = playerRegion[regionId].captainType;//state
		region = &playerRegion[regionId];		
		if (region == NULL) {
			GPTP::logger << "AI Broodling Bug: Null region!" << '\n';
		}
		else {
			if (regionType != 0) //region has assigned state, state 0 is nothing
				AI_AssignMilitary(1, region);
			AI_AddMilitary(region, result, 1);
		}
	}

	// Hooked to investigate crashes - Pr0nogo
/*	void AI_TrainingNormal(CUnit* a1, CUnit* a2) {									// 00435DB0
		s16 v2; // dx
		CUnit* v3; // esi
		s16 regionId; // ax
		UnitAi* v5; // ebx
		UnitAi* v6; // eax
		s32 v7; // edi
		AiTown* v8; // ebx
		AiCaptain* v9; // eax
		s16 v10; // dx
		s16 v11; // cx
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
				&& v2 != UnitId::ProtossDarkArchon
				&& v2 != UnitId::ZergLurker
				&& v2 != UnitId::TerranNuclearMissile
				&& v2 != UnitId::ProtossObserver)
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
						v7 = (u8)a2->buildQueueSlot;
					}
					v9 = (AiCaptain*)*(&v5->field_18 + v7);
					if (*(&v5->field_9 + v7) == 2)
					{
						if (!*(s32*)&v9->field_C)
						{
							v10 = v9->fullAirStrength;
							v11 = v9->field_18;
							*(s32*)&v9->field_C = v3;
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
	void AI_AddUnitAi(CUnit* unit, AiTown* town) { // 00433DD0
		UnitAi* unitAI;
		WorkerAi* unitAIWorker;
		BuildingAi* unitAIBuilding;
		WorkerAiArray* workerAIArray;
		UnitAi* previous1;
		int* buildingQueueValues; 
		u8* buildingQueueTypes; // was char*
		s8 playerRACE;
		s32 playerID;
		u16 unitType;
		
		if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker) {
			workerAIArray = town->free_workers;
			unitAI = (UnitAi*)(workerAIArray->first_free_worker);
			if (unitAI!=nullptr) {
				previous1 = unitAI->link.prev;
				auto prevIsNull = unitAI->link.prev == 0;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (!prevIsNull)
					previous1->link.next = 0;
				if (unitAI != nullptr) {
					
					unitAI->link.next = 0;
					unitAI->link.prev = (UnitAi*)town->workers;
					if (town->workers)
						town->workers->link.next = (WorkerAi*)unitAI;
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
		}
		else if (units_dat::BaseProperty[unit->id] & 1 && unit->id != UnitId::ResourceVespeneGeyser
			|| unit->id == UnitId::ZergLarva
			|| unit->id == UnitId::ZergEgg
			|| unit->id == UnitId::ZergOverlord)
		{
			unitAI = AI_CreateBuildingAi(town->free_buildings);
			if (unitAI) {
				unitAI->type = 3; //building ai
				unitAIBuilding = (BuildingAi*)unitAI;
				unitAIBuilding->parent = unit;
				unitAIBuilding->town = town;
				buildingQueueValues = (int*)unitAIBuilding->trainQueueValues;
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
					)
				{
					if (unit->status & UnitStatus::Completed) {
						orderComputerClear(unit, OrderId::ComputerAI);
						setSecondaryOrder(unit, OrderId::SpreadCreep);
					}
				}
				if (!(AIScriptController[(u8)town->player].AI_Flags.isUseMapSettings) // CONTROLLER_IS_CAMPAIGN
					|| unit->status & UnitStatus::GroundedBuilding
					&& (unit->id != UnitId::TerranMissileTurret)
					&& unit->id != UnitId::TerranBunker
					&& unit->id != UnitId::ZergCreepColony
					&& unit->id != UnitId::ZergSporeColony
					&& unit->id != UnitId::ZergSunkenColony
					&& unit->id != UnitId::ZergLarvalColony
					&& unit->id != UnitId::ProtossPylon
					&& unit->id != UnitId::ProtossPhotonCannon)
				{
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
				}
				else if (playerRACE == RaceId::Protoss) {
					unitType = UnitId::ProtossNexus;
				}
				else {
					unitType = UnitId::ZergHatchery;
				}
				if (unit->id == unitType || unitIsActiveResourceDepot(unit))
				{
					unitAI = (UnitAi*)town->main_building->pAI;
					if (!unitAI)
						town->main_building = unit;
				}
				else {
					auto gas_structure_id = getPlayerDefaultRefineryUnitType(playerID);
					if (unit->id==gas_structure_id || unit->id == UnitId::ResourceVespeneGeyser) {
						if (town->gas_buildings[0]) {
							if (town->gas_buildings[1]) {
								if (!town->gas_buildings[2])
									town->gas_buildings[2] = unit;
							}
							else {
								town->gas_buildings[1] = unit;
							}
						}
						else {
							town->gas_buildings[0] = unit;
						}
						if (town->resource_area)
							town->resource_area_not_set = 1;
					}
				}
			}
		}
	}

	void AI_BuildTechUpgrade(AiTown* town, s32 method, s32 amount, s32 id, s32 priority, s32 need) { // 00431F90
		s32 itr = 0; // eax
		constexpr int BuildCountFlag = 0xF8; //0xf8 are upper flags of 0xff, 0x1/0x2/0x4 are used by other flags
		TownReq* townUnits; // edx
		if (playerTable[(u8)town->player].type == PlayerType::Computer)
		{
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
			}
			else {
				setNeed = true;
			}
			if(setNeed) {
				town->town_units[itr].flags_and_count = need | 0x2 * (method & 0x3 | 0x4 * amount);
				town->town_units[itr].id = id;
				town->town_units[itr].priority = priority;
				// build orders have 0xf8 flag, tech has 0x4, upgrade has 0x2, 0x1 - only if needed
			}
		}
	}

	// Hooked to modify AI behavior such as cancelling structures, withdrawing hurt high-value targets, etc -- Pr0nogo
/*	void AI_UnitWasHit(CUnit* pUnit, CUnit* a2, s32 a3, s32 a4) { // 0043F320
		s8 v4; // cl
		CUnit* v5; // ebx
		CUnit* v6; // esi
		BuildingAi* v7; // eax
		s32 v8; // edx
		s32 quartTotalHP; // edi
		CUnit* v10; // eax
		s32 v11; // eax
		UnitAi* v12; // esi
		s32 v13; // eax
		AiCaptain* v14; // esi
		s32 v15; // edx
		s32 v16; // eax
		AiCaptain* v17; // edi
		char v18; // al
		s16 v19; // ax
		s16 v20; // di
		s32 v21; // esi
		s32 v22; // eax
		s32 v23; // edi
		s32 v24; // eax
		s32 v25; // esi
		s32 v26; // esi
		CUnit* v27; // esi
		u16 v29; // ax
		__int64 v33; // rax
		CUnit* v34; // esi
		s32 v35; // eax
		CUnit* v36; // esi
		CUnit* v37; // eax
		s32 v38; // eax
		s32 v39; // [esp+Ch] [ebp-14h]
		s32 v40; // [esp+Ch] [ebp-14h]
		s32 v41; // [esp+14h] [ebp-Ch]
		s32 v42; // [esp+14h] [ebp-Ch]
		CUnit* a1; // [esp+1Ch] [ebp-4h]
		s32 v44; // [esp+2Ch] [ebp+Ch]

		MilitaryAi* militaryAi;

		v5 = pUnit;
		v4 = pUnit->playerId;
		v6 = a2;
		if (playerTable[v4].type != PlayerType::Computer || v4 >= 8u)
			return;
		if (units_dat::BaseProperty[(u16)v5->id] & UnitProperty::Building)
		{
			if (v5->pAI)
			{
				AIScriptController[v4].lastIndividualUpdateTime = *elapsedTimeSeconds;
				v7 = (BuildingAi*)v5->pAI;
				if (v7)
					v7->town->in_battle = 1;
				if (a3) {
					if (a2) {
						if (units_dat::BaseProperty[(u16)v5->id] & UnitProperty::Worker // cancel structure code
							&& !(v5->mainOrderId) // Original function call checks if unit has a sprite??? 00475A50
							&& !(v5->status & UnitStatus::Completed))
						{
							quartTotalHP = v5->getTotalHitPointsPlusShields() / 4;
							if (v5->getCurrentLifeInGame() < quartTotalHP)
							{
								AI_CancelStructure(v5, v8);
								return;
							}
						}
					}
				}
			}
		}
		v10 = AI_GetActualTarget(v5, v6);
		a1 = v10;
		if (v10->status & UnitStatus::InBuilding)
		{
			a1 = v10->connectedUnit;
			if (!a1)
				return;
		}
		v11 = a1->status;
		if (v11 & 0x800)                            // CloakingForFree
		{
			if (!(v11 & UnitStatus::CloakingForFree))
			{
				a1 = checkArbiterCloakingRange(v5, UnitId::ProtossArbiter);
				if (!a1)
					return;
			}
		}
		if (a3)
		{
			AI_OrderUnitCloaking(v5);
			AI_OrderLurkerBurrow(v5);
		}
		v12 = (UnitAi*)v5->pAI;
		v39 = 0;
		v41 = 0;
		if (!v12)
			goto LABEL_37;
		if (v12->type == 4) {
			militaryAi = (MilitaryAi*)v12;
			if (militaryAi->region->captainType == 1)
			{
				if (v5->isUnitAINonBuilding())
					AI_AssignMilitary(2, militaryAi->region);
			}
			v41 = AI_IsUnderAttack(v5, 0);
		}
		if (!a3 || !unitIsActiveTransport(v5) || !(v5->status & UnitStatus::IsBuilding))
			goto LABEL_37;
		if (isUnitLoaded(v5))
		{
			if (v5->mainOrderId != 111)
				orderComputerClear(v5, 0x6Fu);
			return;
		}
		if (AI_TryReturnHome(v5, 0) || !AI_TransportAction(v5))
		{
			if (AIScriptController[(u8)v5->playerId].AI_Flags.isUseMapSettings)
				v39 = 1;
		LABEL_37:
			v14 = &AiRegionCaptains[(u8)v5->playerId][SAI_GetRegionIdFromPxEx(
				v5->sprite->position.x,
				v5->sprite->position.y)];
			if (!v39)
			{
				if (!a4)
					AI_AskForHelp(v5, a1, v41);
				if (!v41)
				{
					if (!v5->pAI
						|| AI_IsUnderAttack(v5, 0)
						|| (v42 = 1, *(s8*)(v15 + 8) == 1)
						&& AIScriptController[(u8)v5->playerId].AI_Flags.isUseMapSettings)
					{
						v42 = 0;
					}
					v17 = &AiRegionCaptains[(u8)v5->playerId][getRegionIdFromUnit(a1)];
					if (v42)
					{
						v18 = v14->captainType;
						if (v18 == 5 || v18 == 4)
							AI_AssignMilitary(6, v14);
						if (v14->captainType)
						{
							if (!v17->captainType && *elapsedTimeSeconds > 60)
								AI_AssignMilitary(5, v17);
						}
					}
					if (!a4)
					{
						v17->unknown_0x18 = getEnemyAirStrengthInRegion(v17->region, (u8)v5->playerId);
						v17->unknown_0x1A = getEnemyStrengthInRegion(v17->region, (u8)v5->playerId, 0);
						v19 = v17->unknown_0xE;
						if (v19 > v14->unknown_0xE)
							v14->unknown_0xE = v19;
						v20 = v17->unknown_0xC;
						if (v20 > v14->unknown_0xC)
							v14->unknown_0xC = v20;
					}
				}
			}
			v14->captainFlags |= 0x20u;
			if (v5->status & UnitStatus::Completed)
			{
				v21 = (u8)v5->mainOrderId;
				v40 = (u8)v5->mainOrderId;
				if (v21 != OrderId::CompletingArchonSummon && v21 != OrderId::ResetCollision1 && v21 != OrderId::ConstructingBuilding)
				{
					if (a3 || !scbw::orderIsSpell(v21))
					{
						v23 = (u16)v5->id;
						if (v5->id || v21 != OrderId::EnterTransport)
						{
							v44 = unitIsPerformingUnbreakableCode(v5);
							if (v44 || !AI_CastSpellBehavior(v5, a3))
							{
								if (v21 != OrderId::RechargeShields1 && v21 != OrderId::Move)
								{
									if (!a3)
										goto LABEL_126;
									if (v23 != UnitId::ProtossCarrier
										|| (u8)(LOBYTE(v5->field_C8) + BYTE1(v5->carrier.inHangarCount)) > 2u
										|| !AI_TransportAction(v5))
									{
										if (a1 && a1->status & UnitStatus::IsBuilding // Attempt to return to base when below an HP threshold
											|| AIScriptController[(u8)v5->playerId].AI_Flags.isUseMapSettings
											|| ((v25 = 3 * v5->getTotalHitPointsPlusShields() / 4, v5->getCurrentLifeInGame() > v25)
												|| v23 != UnitId::ProtossArbiter && v23 != UnitId::TerranWraith && v23 != UnitId::ZergMutalisk
												|| !AI_TransportAction(v5))
											&& ((v26 = 2 * v5->getTotalHitPointsPlusShields() / 3, v5->getCurrentLifeInGame() > v26)
												|| v23 != UnitId::TerranBattlecruiser && v23 != UnitId::ProtossCarrier && v23 != UnitId::ProtossPanoptus
												|| !AI_TransportAction(v5)))
										{
											if (v5->id != UnitId::ZergLurker
												|| v5->status & UnitStatus::Burrowed
												|| v40 == OrderId::Burrow
												|| v40 == OrderId::Unburrow)
											{
												v27 = a1;
											}
											else {
												v27 = a1;
												if (AI_Flee(v5, a1))
													return;
											}
											if (v44
												|| !(v5->status & UnitStatus::IsBuilding)
												|| unitHasPathToDestOnGround(v5, v27->sprite->position.x, v27->sprite->position.y)
												|| !AI_Flee(v5, v27))
											{
											LABEL_126:
												if (v5->orderTarget.unit != a1) { // Responding with workers
													if (!(units_dat::BaseProperty[(u16)v5->id] & UnitProperty::Worker)
														|| (v29 = a1->id, units_dat::BaseProperty[v29] & UnitProperty::Worker)
														|| v29 == UnitId::ZergZergling
														|| AI_HasNoDefenseBuild((u8)v5->id))
													{
														if (a3) {
															if (v5->pAI)
															{
																if (unitCanSeeCloakedTarget(a1, v5))
																	AI_RespondToCloakedThreat(v5, a1);
															}
														}
														if (!v44)
														{
															if (unitCanAttackTarget(a1, v5, 1))
															{
																v36 = v5->subunit;
																v37 = v5->subunit;
																v5->autoTargetUnit = a1;
																if (v37->isSubunit())
																	v36->autoTargetUnit = a1;
																burrowIdle_Normal(v5, 0, 1, 0);
															}
															else if (v5->pAI)
															{
																if (v40 != 159
																	|| (v33 = AI_PrepareToFlee(a1, v5), (s32)v33 == *(s32*)&v5->sprite->position))
																{
																	if (a3) {
																		if (v5->id != 34)
																		{
																			v34 = v5->orderTarget.unit;
																			if (!v34 || !unitCanAttackTarget(v34, v34, 1))
																				AI_Flee(v5, a1);
																		}
																	}
																}
																else {
																	orderInterrupt(v5, 6u, (Point32*)v33, (CUnit*)HIDWORD(v33)); // point32 was dwPosition, I think that's the same thing
																}
															}
														}
													}
													else if (a3)
													{
														AI_Flee(v5, a1);
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
	}*/

	// Hooked to change guard assignments for structures based on AI personality -- Pr0nogo
	// TODO: Cleanup and make changes
	void AI_IterateBuildingAI(CUnit* result, CUnit* parent) { // 00435770
		UnitAi* parentAI; // eax
		AiTown* parentTown; // esi
		BuildingAi* removeBuilding; // eax
		AiTown* removeBuildingTown; // edi
		CUnit* workerUnit; // esi
		CUnit* workerTarget; // ecx
		s32 queueCount; // eax
		s32 queueCount2;
		BuildingAi* j; // esi
		CUnit* builderUnit; // ecx
		u32 i; // [esp+8h] [ebp-4h]
		WorkerAi* worker;
		BuildingAi* building;

		if (playerTable[(u8)parent->playerId].type == PlayerType::Computer) {
			parentAI = (UnitAi*)parent->pAI;
			if (parentAI) {
				if (parentAI->type == 2) {
					worker = (WorkerAi*)parentAI;
					parentTown = worker->town;
				}
				else { // assumed BuildingAi, unlikely to be GuardAi, can't be anything else
					building = (BuildingAi*)parentAI;
					parentTown = building->town;
				}
				if (parent == result) {
					AI_RemoveFromTowns(result);
					removeBuilding = (BuildingAi*)result->pAI;
					if (removeBuilding) {
						removeBuildingTown = removeBuilding->town;
						AI_DeleteBuildingAI(removeBuilding, removeBuildingTown->free_buildings);
						if (removeBuildingTown)
						{
							if (result == removeBuildingTown->building_scv)
								removeBuildingTown->building_scv = 0;
							if (result == removeBuildingTown->main_building)
								removeBuildingTown->main_building = 0;
						}
						result->pAI = 0;
					}
				}
				AI_AddUnitAi(result, parentTown);
				if (result->id != UnitId::TerranBunker || *elapsedTimeSeconds <= 60)
				{
					if (result->id == UnitId::TerranMissileTurret) {
						workerUnit = parentTown->workers->parent;
						for (i = 0; workerUnit; i += queueCount)
						{
							workerTarget = workerUnit->moveTarget.unit;
							if (workerTarget->id == UnitId::TerranMissileTurret)
								queueCount = 1;
							else
								queueCount = AI_GetQueuedCount(workerTarget, UnitId::TerranMissileTurret);
//							workerUnit = *(u32*)workerUnit; // not sure if this is even necessary, does v8 need to be iterated upon every loop?
						}
						for (j = parentTown->buildings; j; i += queueCount2)
						{
							builderUnit = j->parent;
							if (builderUnit->id == UnitId::TerranMissileTurret)
								queueCount2 = 1;
							else
								queueCount2 = AI_GetQueuedCount(builderUnit, UnitId::TerranMissileTurret);
							j = j->link.prev;
						}
						if (i >= 4) {
							AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 1, result->sprite->position.x);
						}
					}
				}
				else {
					AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 5, result->sprite->position.x);
					AI_PrepareGuardAI(result->sprite->position.y, (u8)result->playerId, 1, result->sprite->position.x);
				}
			}
		}
	}
	
	// hooked for attack extender
//	0x0043E670 => train_attack_force(@eax *mut AiRegion)->u32;
	u32 trainAttackForce(AiCaptain* region) {					// 0043E670
		u32 result; // eax
		u32 v2; // edi
		AiCaptain* v3; // esi
		u16 v4; // bx
		u32 v5; // edi
		u32 v6; // ebx
		u32 v7; // edi
		u32 v8; // ebx
		u32 v9[63]; // [esp+Ch] [ebp-108h]
		u32 v10; // [esp+108h] [ebp-Ch]
		u32 v11; // [esp+10Ch] [ebp-8h]
		u32 v12; // [esp+110h] [ebp-4h]

		v3 = region;
		v2 = (u8)region->playerId;
		result = getMissingAttackForceUnits(region, (s32*)v9, AIScriptController[v2].AI_Flags.isUseMapSettings);
		v10 = result;
		if (AIScriptController[v2].AI_Flags.isUseMapSettings
			|| (v5 = AIScriptController[v2].lastIndividualUpdateTime) != 0
			&& (v12 = *elapsedTimeSeconds - v5 >= 30, *elapsedTimeSeconds - v5 < 30))
		{
			v11 = 60;
		}
		else {
			v11 = 160;
		}
		v6 = 0;
		v12 = 0;
		if (result) {
			do {
				v7 = v9[v6];
				if (v7 < UnitId::None && !AI_AddRegionToMilitary(v3, v7, v11)) {
					v8 = (u8)v3->playerId;
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
	void AIRegion_GroupingForAttack(AiCaptain* region) { // 0043EEC0
		AiCaptain* v1; // esi
		CUnit* v2; // eax
		AiCaptain* v3; // esi
		u32 v4; // eax
		u32 v5; // ecx
		u16 v6; // ax
		AiCaptain* v7; // esi
		AiCaptain* v8; // edi
		u32 v9[63]; // [esp+8h] [ebp-108h]
		s32 a3; // [esp+104h] [ebp-Ch]
		s32 a4; // [esp+108h] [ebp-8h]
		s32 a2; // [esp+10Ch] [ebp-4h]

		AI_RecalculateRegionStrength(region);
		AI_TargetEnemyInRegion(region);
		if (region->captainFlags & 0x20 && AI_MoveMilitaryToDefend(region))
			return;
		AI_GetRegionMilitaryTypes(region, &a2, &a3, &a4);
		v1 = region;
//		if (a2 && SAI_Paths->regions[(u16)region->region].accessabilityFlags == 0x1FFD || AI_CanTravelToRegion(region, region)) // TODO: add SAI_Paths
		{
			AI_AssignMilitary(0, region);
			v2 = AI_PickAttackTarget_NoBuildings(region);
			if (v2) {
				v3 = &AiRegionCaptains[(u8)region->playerId][SAI_GetRegionIdFromPxEx(
					v2->sprite->position.x,
					v2->sprite->position.y)];
				AI_AssignMilitary(9, v3);
				AI_MoveMilitaryToOtherRegion(v3, region, 0, 0, 1);
			}
			else {
				AI_IterateTownSomething(region);
			}
			return;
		}
		if (AI_AttackTimerDecrement((u8)region->playerId)) {
		LABEL_15:
			v6 = region->unknown_0x2;
			if (v6) {
				v7 = &AiRegionCaptains[(u8)region->playerId][v6];
				AI_AssignMilitary(1, v7);
				v8 = v7;
				v1 = region;
				AI_MoveMilitaryToOtherRegion(v8, region, 0, 0, 1);
			}
			AI_AssignMilitary(0, v1);
			return;
		}
		v4 = AI_GetMissingAttackForceUnits(region, (s32*)v9, 1);
		v5 = 0;
		if (!v4) {
		LABEL_14:
			if (AI_HasForcesMovingToAttack((u8)region->playerId))
				return;
			goto LABEL_15;
		}
		while ((u32)v9[v5] >= 0xE4) {
			if (++v5 >= v4)
				goto LABEL_14;
		}
	}

//	0x00438050 => get_missing_attack_force_units_hook(*mut AiRegion,*mut u32,bool)->u32;
/*	u32 AI_GetMissingAttackForceUnits(AiCaptain* region, DWORD* unitIds, u32 isCampaign) {
		s16 v3; // dx
		s16* v4; // ecx
		u32 i; // edi
		u32 v6; // eax
		CUnit* v7; // esi
		s8 v8; // bl
		u32 v9; // ecx
		unitIDs v10; // edx
		s32 v11; // eax
		u32 v12; // esi
		s32 v13; // ecx
		UnitAi* v14; // ecx
		u32 v15; // esi
		u32 v16; // ecx
		CUnit* v18; // [esp+8h] [ebp-4h]
		MilitaryAi* military;

		v4 = AIScriptController[(u8)region->playerId].attackers;
		v3 = *v4;
		for (i = 0; *v4; ++i)
		{
			unitIds[i] = v3 - 1;
			++v4;
			v3 = *v4;
		}
		v6 = (u8)region->playerId;
		v7 = firstPlayerUnit->unit[v6];
		v18 = firstPlayerUnit->unit[v6];
		if (v7) {
			while (1) {
				if (v7->sprite) {
					v8 = v7->mainOrderId;
					if (v8) {
						v9 = (u16)v7->id;
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
			v10 = (u16)v7->id;
			if (v9 == UnitId::TerranSiegeTankSiegeMode)
				v10 = UnitId::TerranSiegeTankTankMode;
			v11 = 1;
			v12 = v7->status & 1;
			if (!v12 || v8 == OrderId::CompletingArchonSummon || v8 == OrderId::ResetCollision1)
			{
				if (isCampaign) {
				LABEL_36:
					v7 = v18;
					goto LABEL_37;
				}
				if (v10 == UnitId::ZergEgg || v10 == UnitId::ZergCocoon) {
					v10 = (u16)v18->buildQueue[(u8)v18->buildQueueSlot];
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
					|| v14 != 0 && v14->type == 4 && military->region == region)
				{
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
	}*/

//	0x00447230 => ai_add_to_attackforce(@edi u32,@eax u32,@ecx u32);
/*	void AI_AddToAttack(u32 result, u32 player, u32 unitId) { // 00447230
		u32 _counter; // esi
		u32 i; // eax
		u32 plOffset; // edx
		u16* atkGroups; // ecx
		u16* v7; // ecx

		_counter = result;
		if (player < 8 && result < 63 && unitId < UnitId::None) {
			plOffset = 628 * player;
			i = 0;
			if (AIScriptController[player].attackers[0]) {
				atkGroups = AIScriptController[player].attackers;
				while (i < 63) {
					++atkGroups;
					++i;
					if (!*atkGroups)
						goto LABEL_8;
				}
			}
			else
			{
			LABEL_8:
				if (_counter)
				{
					v7 = &AIScriptController[0].attackers[plOffset + i];
					do
					{
						--_counter;
						if (i >= 63)
							break;
						*v7 = unitId + 1;
						++i;
						++v7;
					} while (_counter);
				}
			}
		}
	}*/

//	0x0043ABB0 => ai_attack_to_hook(u32,i32,i32,bool,bool)->u32;
	bool AI_AttackManager(u32 _player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack, u32 allowFallbackAirOnly) { // 0043ABB0
		u32 v5; // esi
		WORD v7; // ax
		WORD* v8; // ecx
		char regionsList[2500]; // [esp+4h] [ebp-9C8h]
		u32 v10; // [esp+9C8h] [ebp-4h]

		v5 = _player;
		if (AIScriptController[_player].supplyCollection)
			return 1;
		if (AIScriptController[v5].AI_AttackGroup)
			return *((u8*)&AiRegionCaptains[_player][(u16)AIScriptController[v5].AI_AttackGroup] - 47) != 8;
		memset(regionsList, 0, sizeof(regionsList));
		PopulateRegionsWithOwn(regionsList, _player);
		PopulateRegionsWithEnemy(_player, regionsList);
		while (AI_GetNeighborRegionTypes(regionsList))
			Ai_FinishRegionAccessibility(regionsList);
		v7 = AIScriptController[v5].attackers[0];
		v8 = AIScriptController[v5].attackers;
		v10 = v7 != 0;
		if (v7)
		{
			while (units_dat::DestroyScore[2 * v7 + 226] & 4)
			{
				v7 = v8[1];
				++v8;
				if (!v7)
					goto LABEL_12;
			}
			v10 = 0;
		}
	LABEL_12:
		AIScriptController[v5].AI_Flags.isSecureFinished = 1; // Might be wrong
		if (!AI_SetAttackGroupingRegion(v10, 0, _player, regionsList, x1, y1, alywaysOverwriteLastAttack)
		&& !AI_SetAttackGroupingRegion(v10, 1, _player, regionsList, x1, y1, alywaysOverwriteLastAttack))
		{
			if (!allowFallbackAirOnly || v10) {
				return 1;
			}
			if (!AI_SetAttackGroupingRegion(1, 0, _player, regionsList, x1, y1, alywaysOverwriteLastAttack)
			&& !AI_SetAttackGroupingRegion(1, 1, _player, regionsList, x1, y1, alywaysOverwriteLastAttack))
			{
				AIScriptController[v5].supplyCollection = 1;
				return 1;
			}
		}
		return 0;
	}

//	0x00437F20 => has_units_for_attack_force_hook(@edx u32)->u32;
	s32 AI_HasUnitsForAttackForce(u32 player) { // 00437F20
		u16* v2; // eax
		u16 v3; // cx
		u32 i; // edi
		CUnit* v5; // edx
		u32 v6; // eax
		u8 v7; // bl
		u32 v8; // ecx
		s32 v9; // esi
		u32 v10; // ecx
		u32 v11; // ecx
		u32 v13[63]; // [esp+4h] [ebp-FCh]

		v2 = AIScriptController[player].attackers;
		v3 = *v2;
		for (i = 0; *v2; ++i)
		{
			v13[i] = v3 - 1;
			++v2;
			v3 = *v2;
		}
		v5 = firstPlayerUnit->unit[player];
		if (v5)
		{
			v6 = i - 1;
			do
			{
				if (v5->sprite)
				{
					v7 = v5->mainOrderId;
					if (v7)
					{
						v8 = (u16)v5->id;
						if (!(units_dat::BaseProperty[v8] & 0x10))
						{
							v9 = (u16)v5->id;
							if (v8 == 30)
								v9 = 5;
							if (!(units_dat::ComputerAIInternal[v9] & 1))
							{
								if (v5->status & 1)
								{
									if (v7 != 106 && v7 != 150 && !orders_dat::UseWeaponTargeting[v7])
									{
										if (v5->pAI)
										{
											v10 = 0;
											if (i)
											{
												while (v9 != v13[v10])
												{
													if (++v10 >= i)
														goto LABEL_22;
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
				v5 = v5->player_link.next;
			} while (v5);
		}
		v11 = 0;
		if (!i)
			return 1;
		while (v13[v11] == 228)
		{
			if (++v11 >= i)
				return 1;
		}
		return 0;
	}

//	0x00447040 => ai_remove_from_attack(@eax u32,@edx u32);
	void AI_RemoveFromAttack(s32 unitId, u32 player) { // 00447040
		u32 v2; // esi
		u32 v3; // ecx
		u16* v4; // eax

		if (unitId == UnitId::TerranSiegeTankSiegeMode)
			unitId = UnitId::TerranSiegeTankTankMode;
		v2 = unitId + 1;
		v3 = 0;
		v4 = AIScriptController[player].attackers;
		while (*v4 != v2)
		{
			++v3;
			++v4;
			if (v3 >= 63)
				return;
		}
		AIScriptController[0].attackers[v3 + 628 * player] = 229;
	}
}

namespace {
	// for AI_TrainUnit
	const u32 Func_AI_TrainingWorker = 0x00435700;
	char AI_TrainingWorker(CUnit* result, CUnit* parent) {
		static s32 resultFunc;
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

	const u32 Func_AI_TrainingOverlord = 0x00435770;
	void AI_TrainingOverlord(CUnit* result, CUnit* parent) {
		__asm {
			PUSHAD
			MOV EAX, result
			MOV ECX, parent
			CALL Func_AI_TrainingOverlord
			POPAD
		}
	}

	const u32 Func_AI_TrainingNormal = 0x00435DB0;
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

	const u32 Func_SAI_GetRegionIdFromPxEx = 0x0049C9F0;
	s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EDI, x
			MOV ECX, y
			CALL Func_SAI_GetRegionIdFromPxEx
			MOV result, EAX
			POPAD
		}
		return (s16)result;
	}

	const u32 Func_AI_OrderToDestination = 0x0043D5D0;
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

	const u32 Func_AI_AssignCaptainToSlowestUnit = 0x00436F70;
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region) {
		__asm {
			PUSHAD
			PUSH region
			CALL Func_AI_AssignCaptainToSlowestUnit
			POPAD
		}
	}

	// for AI_CreateNuke

	const u32 Func_createUnit = 0x004A09D0;
	CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId) {
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

	const u32 Func_updateUnitStatsFinishBuilding = 0x004A01F0;
	void updateUnitStatsFinishBuilding(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStatsFinishBuilding
			POPAD
		}
	}

	const u32 Func_updateUnitStrengthAndApplyDefaultOrders = 0x0049FA40;
	u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit) {
		static u32 result;
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_updateUnitStrengthAndApplyDefaultOrders
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 Func_hideUnit = 0x004E6340;
	void hideUnit(CUnit* unit) {
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_hideUnit
			POPAD
		}
	}

	// for AI_TrainingBroodling

	const u32 Func_AI_AssignMilitary = 0x004390A0;
	void AI_AssignMilitary(s32 regionType, AiCaptain* region) {
		__asm {
			PUSHAD
			MOV EBX, regionType
			MOV ESI, region
			CALL Func_AI_AssignMilitary
			POPAD
		}
	}

	const u32 Func_AI_AddMilitary = 0x0043DA20;
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 always_this_region) {
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

	const u32 Func_AI_CreateBuildingAi = 0x004044C0;
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

	const u32 Func_orderComputerClear = 0x00475310;
	void orderComputerClear(CUnit* unit, u8 orderId) {
		__asm {
			PUSHAD
			MOV ESI, unit
			MOVZX CL, orderId
			CALL Func_orderComputerClear
			POPAD
		}
	}

	const u32 Func_setSecondaryOrder = 0x004743D0;
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

	const u32 Func_AI_MakeGuard = 0x0043A010;
	void AI_MakeGuard(s32 playerId, CUnit* unit) {
		__asm {
			PUSHAD
			PUSH unit
			PUSH playerId
			CALL Func_AI_MakeGuard
			POPAD
		}
	}

	const u32 Func_unitIsActiveResourceDepot = 0x00401CF0;
	bool unitIsActiveResourceDepot(CUnit* unit) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EDX, unit
			CALL Func_unitIsActiveResourceDepot
			MOV result, EAX
			POPAD
		}
		return (bool)result;
	}

	const u32 Func_getPlayerDefaultRefineryUnitType = 0x004320D0;
	s32 getPlayerDefaultRefineryUnitType(s32 playerId) {
		static s32 result;
		__asm {
			PUSHAD
			MOV EAX, playerId
			CALL Func_getPlayerDefaultRefineryUnitType
			MOV result, EAX
			POPAD
		}
		return result;
	}
}