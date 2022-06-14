#include "glucustom.h"
#include <hook_tools.h>

namespace
{
	void _declspec(naked) displayEstablishingShot_Wrapper() // 004D9200
	{
		_asm
		{
			PUSHAD
		}
		hooks::displayEstablishingShot();
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDACT_game_speed_Wrapper() // 00481EF0
	{
		static BinDlg* control;
		__asm {
			MOV control, EAX
			PUSHAD
		}
		hooks::CMDACT_game_speed(control);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_glucustom_hooks()
	{
		jump_patch(displayEstablishingShot_Wrapper,		0x004D9200, 1);
		jump_patch(CMDACT_game_speed_Wrapper,			0x00481EF0, 0);
	}
}