#include "stim_packs.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) cmdact_stimpack_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::cmdact_stimpack();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) use_stimpack_ai_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::use_stimpack_ai(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_stimpack_wrapper()
	{
		__asm
		{
			PUSHAD
		}
		hooks::cmdrecv_stimpack();
		__asm {
			POPAD
			RETN
		}
	}

}

namespace hooks
{
	void injectStimPacksHooks()
	{
		jump_patch(cmdact_stimpack_wrapper,		0x004234D0, 4);
		jump_patch(use_stimpack_ai_wrapper,		0x004554A0, 5);
		jump_patch(cmdrecv_stimpack_wrapper,	0x004C2F30, 3);
	}
}
