#include "glucustom.h"
#include <hook_tools.h>

namespace
{
	void _declspec(naked) display_establishing_shot_wrapper() // 004D9200
	{
		_asm
		{
			PUSHAD
		}
		hooks::display_establishing_shot();
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDACT_game_speed_wrapper() // 00481EF0
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, EAX
			PUSHAD
		}
		hooks::cmdact_game_speed(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) hide_dialog_wrapper() // 00418700
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, ESI
			PUSHAD
		}
		hooks::cmdact_game_speed(dialog);
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
		jump_patch(display_establishing_shot_wrapper,	0x004D9200, 1);
		jump_patch(CMDACT_game_speed_wrapper,			0x00481EF0, 0);
	//	jump_patch(hide_dialog_wrapper,					0x00418700, 1); // Not actually necessary for the change.
	}
}