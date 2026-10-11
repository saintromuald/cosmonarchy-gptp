#include "CMDRECV_Stop.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_reaver_stop_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::cmdrecv_reaver_stop();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) cmdrecv_carrier_stop_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::cmdrecv_carrier_stop();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) cmdrecv_stop_wrapper()
	{
		static u8 bCommandType;
		__asm {
			MOV AL, [EDI + 0x01]
			MOV bCommandType, AL
			PUSHAD
		}
		hooks::cmdrecv_stop(bCommandType);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_cmdrecv_stop_hooks()
	{
		jump_patch(cmdrecv_reaver_stop_wrapper,		0x004C1240, 1);
		jump_patch(cmdrecv_carrier_stop_wrapper,	0x004C1430, 1);
		jump_patch(cmdrecv_stop_wrapper,			0x004C2190, 3);
	}
}