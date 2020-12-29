#include "ai_orders.h"
#include <hook_tools.h>

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
}

namespace hooks {

	void injectAIOrdersHooks() {
//		jmpPatch(AI_TrainingUnit_Wrapper,		0x004A2830, 2);
//		jmpPatch(AI_CreateNuke_Wrapper,			0x0045B7A0, 0);
		jmpPatch(AI_TrainingBroodling_Wrapper,	0x0043E280, 1);
	}
}