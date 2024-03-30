#include "hotkeys.h"
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
		result = 0; // We handle this ourselves in GPTP, we don't need to handle it twice
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
		result = 0; // We handle this ourselves in GPTP, we don't need to handle it twice
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

	void __declspec(naked) game_input_user_key_press_wrapper() { // 004846B0
		static DialogEvent* event_arg;
		static u8 result;
		__asm {
			MOV event_arg, ECX
			PUSHAD
		}
		result = hooks::game_input_user_key_press(event_arg);
		__asm {
			POPAD
			MOV result, AL
			RET
		}
	}
}

namespace hooks {
	void inject_hotkey_hooks() {
		if (g_config.extended_camera_hotkeys) {
			jump_patch(save_screen_location_wrapper,		0x00484360, 0);
			jump_patch(recall_screen_location_wrapper,		0x00484500, 1);
			jump_patch(help_menu_call_wrapper,				0x00484770, 0);
			jump_patch(screen_location_switch_case_wrapper,	0x0048478A, 1);
			jump_patch(game_input_user_key_press_wrapper,	0x004846B0, 0);

			// Remove Ctrl-R
			if (g_config.extended_control_groups) {
				nops(0x00484785, 5);
			}
		}
	}
}