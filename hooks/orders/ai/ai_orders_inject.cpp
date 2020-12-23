#include "ai_orders.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) AI_TrainingUnit_Wrapper() { // 004A2830
		static CUnit* result;
		static CUnit* parent;
		__asm {
			PUSHAD
			MOV result, EAX
			MOV parent, ECX
		}
		hooks::AI_TrainingUnit(result, parent);
		__asm {
			POPAD
		}
	}
}

namespace hooks {

	void injectAIOrdersHooks() {
		jmpPatch(AI_TrainingUnit_Wrapper, 0x004A2830, 2);
	}
}