#include "console.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) loadRaceUI_Wrapper() // 004EE2D0
	{
		static s32 result;
		__asm {
			PUSHAD
		}
		result = hooks::loadRaceUI();
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}

	void __declspec(naked) setTextStr_Wrapper() // 004263E0
	{
		static BinDlg* a1;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a1, EAX
			POP EAX
			PUSHAD
		}
		hooks::setTextStr(a1);
		_asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void injectConsoleHooks()
	{
		jump_patch(loadRaceUI_Wrapper, 0x004EE2D0, 3);
		jump_patch(setTextStr_Wrapper, 0x004263E0, 0);
	}
}