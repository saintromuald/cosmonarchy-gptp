#include "screen_hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>

// TODO: move to helpers
s32 move_screen(s32 x, s32 y);
void centerview_unit_group(u8 index);
s32 select_unit_group(u8 index);

Point16 screen_hotkeys[CameraCap];

void init_screen_locations() {
	for (u32 i = 0; i < CameraCap; ++i) {
		hooks::save_screen_location(i);
	}
}

void handle_screen_hotkeys() {
	// Check if the window is in focus
	if (GetActiveWindow() != *g_main_window) {
		return;
	}
	if (*g_is_game_paused) {
		// Only fetch keys if paused
		for (s32 i = 0; i < CameraCap; i++) {
			GetAsyncKeyState(VK_F1 + i);
		}
		return;
	}
	// Fetch and process keys if unpaused
	for (s32 i = 0; i < CameraCap; i++) {
		// Unfortunately `g_is_keycode_used` doesn't fetch the F keys,
		// using that would have been way easier as it's current frame specific and already checks if the game is in focus
		if (GetAsyncKeyState(VK_F1 + i)) {
			if (*g_is_holding_ctrl || *g_is_holding_shift) {
				hooks::save_screen_location(i);
			} else {
				hooks::recall_screen_location(i);
			}
			break;
		}
	}
}

namespace hooks {
	s32 save_screen_location(s32 index) {
		screen_hotkeys[index].x = *g_move_to_x;
		screen_hotkeys[index].y = *g_move_to_y;
		return index;
	}

	s32 recall_screen_location(s32 index) {
		return move_screen(screen_hotkeys[index].x, screen_hotkeys[index].y);
	}

	bool screen_location_switch_case(s32 n) {
		switch (n) {
			case 169:
			//	save_screen_location(0);
				break;
			case 170:
			//	save_screen_location(1);
				break;
			case 171:
			//	save_screen_location(2);
				break;
			case 172:
			//	recall_screen_location(0);
				break;
			case 173:
			//	recall_screen_location(1);
				break;
			case 174:
			//	recall_screen_location(2);
				break;
			default:
				return false;
		}
		return true;
	}
}

s32 move_screen(s32 x, s32 y) {
	const u32 func_address = 0x0049C440;
	static s32 result;
	_asm {
		PUSHAD
		MOV EAX, x
		MOV ECX, y
		CALL func_address
		MOV result, EAX
		POPAD
	}
	return result;
}


void centerview_unit_group(u8 index) {
	const u32 address_func = 0x004967E0;
	_asm {
		PUSHAD
		PUSH index
		CALL address_func
		POPAD
	}
}

s32 select_unit_group(u8 index) {
	const u32 address_func = 0x00496B40;
	static s32 result;
	_asm {
		PUSHAD
		PUSH index
		CALL address_func
		MOV result, EAX
		POPAD
	}
	return result;
}