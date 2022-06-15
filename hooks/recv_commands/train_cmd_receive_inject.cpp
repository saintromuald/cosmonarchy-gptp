#include "train_cmd_receive.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_train_fighter_wrapper()
	{
		__asm PUSHAD
		hooks::cmdrecv_train_fighter();
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_train_2_wrapper()
	{
		static u16 unit_id;
		__asm
		{
			MOV AX, [EAX + 01]
			MOV unit_id, AX
			PUSHAD
		}
		hooks::cmdrecv_train(unit_id);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_train_cmdrecv_hooks()
	{
		jump_patch(cmdrecv_train_fighter_wrapper,	0x004C1800, 5);
	//	jump_patch(cmdrecv_train_2_wrapper,		 	0x004C1C20, 7);
	}
}