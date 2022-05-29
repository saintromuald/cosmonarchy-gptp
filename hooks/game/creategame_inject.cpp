#include "creategame.h"
#include <hook_tools.h>
/*
namespace {
	__int16 __declspec(naked) singleplayerMeleeCreate_Wrapper() { //004EE110
		static __int16 result;
		__asm {
			PUSHAD
		}
		result = hooks::singleplayerMeleeCreate();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};

	s32 loadGameCreate_Wrapper(s32 a1, char a2) { //004EE520
		static s32 a1;
		static char a2;
		static s32 result;
		__asm {
			MOV a1, EAX
			MOV a2, EDI
			PUSHAD
		}
		result = hooks::loadGameCreate();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};

	s32 levelCheatCreate_Wrapper(char a1, s32 a2) { //004EE5B0
		static char a1;
		static s32 a2;
		static s32 result;
		__asm {
			MOV a1, EDI
			MOV a2, ESI
			PUSHAD
		}
		result = hooks::levelCheatCreate();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	};
}

namespace hooks {

	void injectCreategameHooks() {
		jump_patch(singleplayerMeleeCreate_Wrapper, 0x004EE110, 1);
		jump_patch(loadGameCreate_Wrapper, 0x004EE520, 1);
		jump_patch(levelCheatCreate_Wrapper, 0x004EE5B0, 1);
	}
}*/