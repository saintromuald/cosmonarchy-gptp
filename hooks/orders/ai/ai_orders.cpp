#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace {
	// for AI_TrainingUnit
	char AI_TrainingWorker(CUnit* result, CUnit* parent);						// 00435700
	void AI_TrainingOverlord(CUnit* result, CUnit* parent);						// 00435770
	void AI_TrainingBroodling(CUnit* parent, CUnit* result);					// 0043E280
	void AI_TrainingNormal(CUnit* result, CUnit* parent);						// 00435DB0

	// for AI_AddMilitary
	s16 SAI_GetRegionIdFromPxEx(s32 x, s32 y);									// 0049C9F0
	void AI_OrderToDestination(CUnit* unit, int orderId, int x, int y);			// 0043D5D0
	void AI_AssignCaptainToSlowestUnit(AiCaptain* region);						// 00436F70
}

namespace hooks {
	void AI_TrainingUnit(CUnit* result, CUnit* parent) {						// 004A2830
		if (playerTable[result->playerId].type == PlayerType::Computer) {
			if (playerTable[result->playerId].type == PlayerType::Computer
				&& result->id == UnitId::TerranMedic) {
				if (!(AIScriptController[result->playerId].mainMedic)) {
					AIScriptController[result->playerId].mainMedic = result;
				}
			}

			if (result->id == UnitId::ZergBroodling
			|| result->id == UnitId::ZergBroodlisk
			//|| result->id == UnitId::ProtossSimulacrum
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
	/*
	void AI_AddMilitary(AiCaptain* region, CUnit* unit, s32 alwaysThisRegion) {	// 0043DA20
	// alwaysThisRegion acts as a bool, not sure why IDA thinks it's s32
	// I assumed medicAI was UnitAI* but IDA tries to assign town values to it later
	// Looking at the aise version of this hook didn't help as it's more akin to a rewrite: https://pastebin.com/raw/8xNTUCuv
		AiCaptain* regionNew; // esi
		UnitAI* medicAI; // eax
		CUnit* medic; // edx
		UnitAI* nextAI; // ecx
		bool previousBool; // zf
		AiTown* AItown; // ecx

		regionNew = region;
		if (region->captainType == 3 && !alwaysThisRegion) {
			regionNew = &AiRegionCaptains[(u8)unit->playerId][SAI_GetRegionIdFromPxEx(
				unit->sprite->position.x,
				unit->sprite->position.y)];
		}
		medic = region->mainMedic;
		medicAI = (UnitAI*)medic->pAI;
		if (medicAI) {
			nextAI = (UnitAI*)medicAI->link.prev;
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
					AssignCaptainToSlowestUnit(v3);
			}
		}
	}*/
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

	const u32 Func_AI_TrainingBroodling = 0x0043E280;
	void AI_TrainingBroodling(CUnit* parent, CUnit* result) {
		__asm {
			PUSHAD
			PUSH result
			PUSH parent
			CALL Func_AI_TrainingBroodling
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

}