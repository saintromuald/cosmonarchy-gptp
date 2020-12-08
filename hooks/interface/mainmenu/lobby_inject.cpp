#include "lobby.h"
#include <hook_tools.h>

namespace { // Wrappers

	void __declspec(naked) initGame_Wrapper() { // 004EE110
		static s32 result;
		__asm {
			PUSHAD
		}
		result = hooks::initGame();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}

}

namespace hooks {

	void injectLobbyHooks() {
		jmpPatch(initGame_Wrapper, 0x004EE110, 1);
	}
}