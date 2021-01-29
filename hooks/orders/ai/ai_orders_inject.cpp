#include "ai_orders.h"
#include <hook_tools.h>
#include "SCBW/api.h"
namespace {

	void __declspec(naked) AI_TrainingUnit_Wrapper() { // 004A2830
		static CUnit* result;
		static CUnit* parent;
		__asm {
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::AI_TrainingUnit(result, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_CreateNuke_Wrapper() { // 0045B7A0
		static CUnit* silo;
		static CUnit* result;
		__asm {
			MOV silo, EDI
			MOV result, EAX
			PUSHAD
		}
		hooks::AI_CreateNuke(silo, result);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_TrainingBroodling_Wrapper() { // 0043E280
		static CUnit* parent;
		static CUnit* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV parent, EAX
			MOV EAX, [EBP + 0xC]
			MOV result, EAX
			POP EAX
			PUSHAD
		}
		hooks::AI_TrainingBroodling(parent, result);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) AI_AddUnitAi_Wrapper() { // 00433DD0
		static CUnit* unit;
		static AiTown* town;
		__asm {
			MOV unit, EBX
			MOV town, EDI
			PUSHAD
		}
		hooks::AI_AddUnitAi(unit, town);
		__asm {
			POPAD
			RETN
		}
	}

	const u32 addUnitAiShortJump = 0x00433DED;
	void __declspec(naked) AI_AddUnitAi_FreeWorkerCheck_JmpWrapper() {
		static WorkerAiArray* free;
		__asm {
			MOV free,EDX
			PUSHAD
		}
		if ((u32)free <= 0x100000) {
			scbw::printText("Corrupted pointer in AddUnitAi!");
			__asm {
				POPAD
				POP ESI
				RETN
			}
		}
		else {
			__asm {
				POPAD
				MOV EAX, [EDX + 0x5DC0]
				JMP addUnitAiShortJump
			}
		}
		
	}

	void __declspec(naked) AI_BuildTechUpgrade_Wrapper() { // 00431F90
		static AiTown* town;
		static s32 method;
		static s32 amount;
		static s32 id;
		static s32 priority;
		static s32 need;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV method, EAX
			MOV EAX, [EBP + 0xC]
			MOV amount, EAX
			MOV EAX, [EBP + 0x10]
			MOV id, EAX
			MOV EAX, [EBP + 0x14]
			MOV priority, EAX
			MOV EAX, [EBP + 0x18]
			MOV need, EAX
			POP EAX
			MOV town, ESI
			PUSHAD
		}
		hooks::AI_BuildTechUpgrade(town, method, amount, id, priority, need);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 20
		}
	}

	void __declspec(naked) trainAttackForce_Wrapper() { // 0043E670
		static AiCaptain* region;
		static u32 result;
		__asm {
			MOV region, EAX
			PUSHAD
		}
		result = hooks::trainAttackForce(region);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}
}

namespace hooks {

	void injectAIOrdersHooks() {
		jmpPatch(AI_TrainingUnit_Wrapper,		0x004A2830, 2);
//		jmpPatch(AI_CreateNuke_Wrapper,			0x0045B7A0, 0);
		jmpPatch(AI_TrainingBroodling_Wrapper,	0x0043E280, 1);
//		jmpPatch(AI_AddUnitAi_Wrapper,			0x00433DD0, 2);
		jmpPatch(AI_AddUnitAi_FreeWorkerCheck_JmpWrapper, 0x00433DE7, 1);
//		jmpPatch(AI_BuildTechUpgrade_Wrapper,	0x00431F90, 2);
//		jmpPatch(trainAttackForce_Wrapper,		0x0043E670, 4);
	}
}