#include "sfxhooks.h"
#include <hook_tools.h>

namespace {

	int __declspec(naked) getResponseSfx_Wrapper() { // 0048EA20
		static signed int a1;
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
	};

	int __declspec(naked) getRandYesSFX_Wrapper() { // 0048EAB0
		static signed int a1;
		static int result;
		__asm {
			PUSH a1
			PUSHAD
		}
		result = hooks::getRandYesSFX(a1);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};
}

namespace hooks {

	void injectSfxHooks() {
		jmpPatch(getResponseSfx_Wrapper, 0x0048EA20, 1);
		jmpPatch(getRandYesSFX_Wrapper, 0x0048EAB0, 1);
	}
}