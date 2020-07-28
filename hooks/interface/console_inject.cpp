#include "console.h"
#include <hook_tools.h>

namespace {
	__int16 __declspec(naked) loadRaceUI_Wrapper() { //004EE2D0
		static int result;
		__asm {
			PUSHAD
		}
		result = hooks::loadRaceUI();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};
}

namespace hooks {

	void injectConsoleHooks() {
		jmpPatch(loadRaceUI_Wrapper, 0x004EE2D0, 1);
	}
}