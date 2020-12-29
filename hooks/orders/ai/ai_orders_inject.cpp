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
}

namespace hooks {

	void injectAIOrdersHooks() {
//		jmpPatch(AI_TrainingUnit_Wrapper,	0x004A2830, 2);
//		jmpPatch(AI_CreateNuke_Wrapper,		0x0045B7A0, 0);
	}
}