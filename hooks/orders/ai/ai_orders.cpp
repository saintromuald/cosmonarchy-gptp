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
				if (parent->pAI != nullptr) {
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
			if (regionType != 0)//region has assigned state, state 0 is nothing
				AI_AssignMilitary(1, region);
			AI_AddMilitary(region, result, 1);
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
}