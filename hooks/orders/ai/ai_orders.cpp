#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace {
	// for AI_TrainingUnit
	char AI_TrainingWorker(CUnit* result, CUnit* parent);							// 00435700
	void AI_TrainingOverlord(CUnit* result, CUnit* parent);							// 00435770
	void AI_TrainingNormal(CUnit* result, CUnit* parent);							// 00435DB0

	// for AI_AddMilitary
	s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);										// 0049C9F0
	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y);				// 0043D5D0
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region);							// 00436F70

	// for AI_CreateNuke
	CUnit* createUnit(s16 unitType, s32 x, s32 y, s32 playerId);					// 004A09D0
	void updateUnitStatsFinishBuilding(CUnit* unit);								// 004A01F0
	u32 updateUnitStrengthAndApplyDefaultOrders(CUnit* unit);						// 0049FA40
	void hideUnit(CUnit* unit);														// 004E6340

	// for AI_TrainingBroodling
	void AI_AssignMilitary(s32 regionType, AiCaptain* region);						// 004390A0
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 always_this_region);	// 0043DA20

	// for AI_AddUnitAi
	UnitAi* AI_CreateBuildingAi(BuildingAiArray* buildingAI);						// 004044C0
	void orderComputerClear(CUnit* unit, u8 orderId);								// 00475310
	CUnit* setSecondaryOrder(CUnit* result, char orderId);							// 004743D0
	void AI_MakeGuard(s32 playerId, CUnit* unit);									// 0043A010
	bool unitIsActiveResourceDepot(CUnit* unit);									// 00401CF0
	s32 getPlayerDefaultRefineryUnitType(s32 playerId);								// 004320D0

}
#include "logger.h"
namespace hooks {
	void AI_TrainingUnit(CUnit* result, CUnit* parent) {							// 004A2830
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
						AI_TrainingOverlord(result, parent);
					}
					else {
						AI_TrainingNormal(result, parent);
					}
				}
				
			}
		}
	}
	
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
			unitAI = (UnitAi*)workerAIArray->first_free_worker;
			if (unitAI) {
				previous1 = unitAI->link.prev;
				workerAIArray->first_free_worker = (WorkerAi*)unitAI->link.prev;
				if (!unitAI->link.prev)
					previous1->link.next = 0;
				if (unitAI) {
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
				unitAI->type = 3;//building ai
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
					|| unit->id == UnitId::ZergSire //new, hydra
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
					&& unit->id != UnitId::ZergLarvalColony //new, hydra
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