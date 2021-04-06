#include "player_color.h"
#include <hook_tools.h>

namespace {

	/*	void __declspec(naked) AI_TrainingUnit_Wrapper() { // 004A2830
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
		}*/
}

namespace hooks {

	void injectPlayerColorHooks() {
		//		jmpPatch(AI_TrainingUnit_Wrapper, 0x004A2830, 2);
	}
}