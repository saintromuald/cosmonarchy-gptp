#include "hpbars.h"
#include "hook_tools.h"

namespace
{
	void _declspec(naked) compileHealthBar_Wrapper()
	{
		static Image* a1;
		static Sprite* a2;
		_asm {
			MOV a1, ESI
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a2, EAX
			POP EAX
			PUSHAD
		}
		hooks::compileHealthBar(a1, a2);
		_asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectHpBarsHooks()
	{
		//jump_patch(compileHealthBar_Wrapper, 0x004D6010, 2);
	}
}