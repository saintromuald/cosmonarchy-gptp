#include "CMDRECV_SiegeTank.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) CMDRECV_siege_wrapper()
	{
		static u8 is_queued;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV AL, [EAX + 1]
			MOV is_queued, AL
			PUSHAD
		}
		hooks::CMDRECV_Siege(is_queued);
		__asm {
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) CMDRECV_unsiege_wrapper()
	{
		static u8 is_queued;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV AL, [EAX + 1]
			MOV is_queued, AL
			PUSHAD
		}
		hooks::CMDRECV_Unsiege(is_queued);
		__asm {
			POPAD
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_cmdrecv_siege_tank_hooks()
	{
		jump_patch(CMDRECV_siege_wrapper,	0x004C1E80, 6);
		jump_patch(CMDRECV_unsiege_wrapper,	0x004C1F10, 6);
	}
}