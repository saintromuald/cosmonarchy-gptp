#include "CMDRECV_Cancel.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) CMDRECV_CancelNukeWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelNuke();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDRECV_CancelAddonWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelAddon();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDRECV_CancelUpgradeWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelUpgrade();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDRECV_CancelResearchWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelResearch();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDRECV_CancelTrainWrapper()
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
		hooks::CMDRECV_CancelTrain(wUnitID);
		__asm {
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) CMDRECV_CancelUnitMorphWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelUnitMorph();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDRECV_CancelConstructionWrapper()
	{
		__asm {
			PUSHAD
		}
		hooks::CMDRECV_CancelConstruction();
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
		jump_patch(CMDRECV_CancelNukeWrapper,			0x004BFCD0, 4);
		jump_patch(CMDRECV_CancelAddonWrapper,			0x004BFF30, 3);
		jump_patch(CMDRECV_CancelUpgradeWrapper,		0x004BFFC0, 3);
		jump_patch(CMDRECV_CancelResearchWrapper,		0x004C0070, 3);
		jump_patch(CMDRECV_CancelTrainWrapper,			0x004C0100, 6);
		jump_patch(CMDRECV_CancelUnitMorphWrapper,		0x004C2EC0, 2);
		jump_patch(CMDRECV_CancelConstructionWrapper,	0x004C2EF0, 3);
	}
}