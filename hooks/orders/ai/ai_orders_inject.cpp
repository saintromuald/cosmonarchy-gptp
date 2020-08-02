#include "ai_orders.h"
#include <hook_tools.h>

namespace {

	int __declspec(naked) askForHelp_AI_Wrapper() { // 0043ADA0
		static CUnit* a1;
		static CUnit* a2;
		static int a3;
		static int result;
		__asm {
			PUSHAD
		}
		result = hooks::askForHelp_AI(a1, a2, a3);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};
}

namespace hooks {

	void injectAIOrdersHooks() {
		jmpPatch(askForHelp_AI_Wrapper, 0x0043ADA0, 1);
	}
}