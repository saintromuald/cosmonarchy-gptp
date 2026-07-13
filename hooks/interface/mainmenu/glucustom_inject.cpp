#include "glucustom.h"
#include <hook_tools.h>

namespace {
	void _declspec(naked) display_establishing_shot_wrapper() { // 004D9200
		_asm {
			PUSHAD
		}
		hooks::display_establishing_shot();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) CMDACT_game_speed_wrapper() { // 00481EF0
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

	void __declspec(naked) hide_dialog_wrapper() { // 00418700
		static BinDlg* dialog;
		__asm {
			MOV dialog, ESI
			PUSHAD
		}
		hooks::hide_dialog(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) load_title_wrapper() { // 004D49A0
		__asm {
			PUSHAD
		}
		hooks::load_title();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) create_main_window_wrapper() { // 004D49A0
		__asm {
			PUSHAD
		}
		hooks::create_main_window();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) initialize_game_type_list_wrapper() { // 004AF400
		static BinDlg* list;
		__asm {
			MOV list, EAX
			PUSHAD
		}
		hooks::initialize_game_type_list(list);
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
	//	jump_patch(load_title_wrapper,					0x004D49A0, 1); // Crashes related to alloc background image.
	//	jump_patch(create_main_window_wrapper,			0x004E05B0, 1); // Does not support Xenotron wmode.
		jump_patch(initialize_game_type_list_wrapper,	0x004AF400, 0); // Not tested yet.
	}
}