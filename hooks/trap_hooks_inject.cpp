#include "trap_hooks.h"
#include "hook_tools.h"
#include "SCBW/api.h"

namespace {
	void _declspec(naked) ReturnFalse_Wrapper() {
		_asm {
			PUSHAD
		}
		scbw::printText("u");
		_asm {
			POPAD
		}
		_asm {
			MOV EAX, 0
			RETN
		}
	}
}

namespace hooks {
	void injectTrapHooks() {
		jmpPatch(ReturnFalse_Wrapper, 0x00401430, 3);
		jmpPatch(ReturnFalse_Wrapper, 0x0047B070, 3);
	}
}