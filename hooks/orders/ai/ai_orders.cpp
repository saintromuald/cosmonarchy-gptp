#include "ai_orders.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>
#include "hooks\byte_operations_utils.h"

namespace {
	// for AI_TrainingUnit
	char AI_TrainingWorker(CUnit* result, CUnit* parent);			// 00435700
	void AI_TrainingOverlord(CUnit* result, CUnit* parent);			// 00435770
	void AI_TrainingBroodling(CUnit* parent, CUnit* result);		// 0043E280
	void AI_TrainingNormal(CUnit* result, CUnit* parent);			// 00435DB0
}

namespace hooks {
	void AI_TrainingUnit(CUnit* result, CUnit* parent) {		// 004A2830
		int v2; // edx
		int v3; // edx
		int v4; // esi
		CUnit* v5; // edx

		if (playerTable[(u8)parent->playerId].type == PlayerType::Computer) {
			if (playerTable[(u8)result->playerId].type == PlayerType::Computer
			&&  result->id == UnitId::TerranMedic) {
				if (!(AIScriptController[(u8)result->playerId].mainMedic))
					  AIScriptController[(u8)result->playerId].mainMedic = result;
			}
			if (units_dat::BaseProperty[result->id] & UnitProperty::Worker) {
				AI_TrainingWorker(result, parent);
			}
			else if (result->id == UnitId::ZergOverlord) {
				AI_TrainingOverlord(result, parent);
			}
			else if (result->id == UnitId::ZergBroodling
			||		 result->id == UnitId::ZergBroodlisk
			||		 result->id == UnitId::ProtossSimulacrum) {
				AI_TrainingBroodling(parent, result);
			}
			else {
				AI_TrainingNormal(result, parent);
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
		return (char)resultFunc;
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

}