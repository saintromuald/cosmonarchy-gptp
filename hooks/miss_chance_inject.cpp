#include "miss_chance.h"
#include <hook_tools.h>

namespace {

	int __declspec(naked) isUnderDarkSwarm_Wrapper() { // 00476210
		static CUnit* a1;
		static int result;
		__asm {
			PUSHAD
			MOV a1, EAX
		}
		result = hooks::isUnderDarkSwarm(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	};

	int __declspec(naked) canHitTargetCheck_Wrapper() { // 004765B0
		static CUnit* a1;
		static CUnit* a2;
		static int result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, ECX
		}
		result = hooks::canHitTargetCheck(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	};
}

namespace hooks {

	void injectMissChanceHooks() {
		//jmpPatch(isUnderDarkSwarm_Wrapper, 0x00476210, 1);
		jmpPatch(canHitTargetCheck_Wrapper, 0x004765B0, 1);
	}
}