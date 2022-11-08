#include "screen_hotkeys.h"
#include "hook_tools.h"
#include "config.h"

namespace {

	void _declspec(naked) save_screen_location_wrapper() {
		static s32 id;
		static s32 result;
		_asm {
			MOV id, EAX
			PUSHAD
		}
		result = hooks::save_screen_location(id);
		_asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void _declspec(naked) recall_screen_location_wrapper() {
		static s32 id;
		static s32 result;
		_asm {
			MOV id, EAX
			PUSHAD
		}
		result = hooks::recall_screen_location(id);
		_asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void _declspec(naked) help_menu_call_wrapper() {
		static const u32 jmp_address = 0x0048478A;
		_asm {
			JMP jmp_address
		}
	}

	void _declspec(naked) screen_location_switch_case_wrapper() {
		static s32 v1;
		static bool result;
		_asm {
			MOV v1, EDI
			PUSHAD
		}
		if (!*g_is_game_paused) {
			result = hooks::screen_location_switch_case(v1 + 0x642D);
			if (!result) {
				static const u32 screenLocationSwitchCase_jmp = 0x00484796;
				_asm { // continue executing switch case if case wasn't found
					POPAD
					JMP screenLocationSwitchCase_jmp
				}
			}
		}
		static const u32 loc_484B24 = 0x00484B24;
		_asm { // return
			POPAD
			JMP loc_484B24
		}
	}
}

namespace hooks {
	void inject_screen_hotkey_hooks() {
		if (config.extended_camera_hotkeys) {
			jump_patch(save_screen_location_wrapper,		0x00484360, 0);
			jump_patch(recall_screen_location_wrapper,		0x00484500, 1);
			jump_patch(help_menu_call_wrapper,				0x00484770, 0);
			jump_patch(screen_location_switch_case_wrapper,	0x0048478A, 1);
		}
	}
}