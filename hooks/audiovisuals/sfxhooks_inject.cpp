#include "sfxhooks.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) getResponseSfx_Wrapper() { //0048EA20
		static int a1;
		static int a2;
		static int result;
		__asm {
			MOV a1, EAX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::getResponseSfx(a1, a2);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}
	;
}

namespace hooks {

	void injectSfxHooks() {
		jmpPatch(getResponseSfx_Wrapper, 0x0048EA20, 1);
	}
}