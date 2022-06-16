#include "CMDRECV_Cancel.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_cancel_nuke_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_nuke();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_cancel_addon_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_addon();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_cancel_upgrade_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_upgrade();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_cancel_research_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_research();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_cancel_train_wrapper()
	{
		static u16 wUnitID;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV AX, [EAX + 0x01]
			MOV wUnitID, AX
			PUSHAD
		}
		hooks::cmdrecv_cancel_train(wUnitID);
		__asm {
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) cmdrecv_cancel_unit_morph_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_unit_morph();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_cancel_construction_wrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::cmdrecv_cancel_construction();
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_cmdrecv_cancel_hooks()
	{
		jump_patch(cmdrecv_cancel_nuke_wrapper,			0x004BFCD0, 4);
		jump_patch(cmdrecv_cancel_addon_wrapper,		0x004BFF30, 3);
		jump_patch(cmdrecv_cancel_upgrade_wrapper,		0x004BFFC0, 3);
		jump_patch(cmdrecv_cancel_research_wrapper,		0x004C0070, 3);
		jump_patch(cmdrecv_cancel_train_wrapper,		0x004C0100, 6);
		jump_patch(cmdrecv_cancel_unit_morph_wrapper,	0x004C2EC0, 2);
		jump_patch(cmdrecv_cancel_construction_wrapper,	0x004C2EF0, 3);
	}
}