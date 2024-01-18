#include "hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>
#include "hooks/interface/buttonsets.h"

#define CameraCap 8
static Point16 screen_hotkeys[CameraCap];

Keyboard keyboard;

void init_hotkeys() {
	keyboard = {};
	for (u32 i = 0; i < CameraCap; ++i) {
		screen_hotkeys[i].x = *g_move_to_x;
		screen_hotkeys[i].y = *g_move_to_y;
	}
}

void update_hotkeys() {
	keyboard.update();

	if (config.extended_camera_hotkeys) {
		for (s32 i = 0; i < CameraCap; i++) {
			if (keyboard.held(VK_F1 + i)) {
				if (keyboard.held(VK_CONTROL) || keyboard.held(VK_SHIFT)) {
					screen_hotkeys[i].x = *g_move_to_x;
					screen_hotkeys[i].y = *g_move_to_y;
				} else {
					helper::move_screen(screen_hotkeys[i]);
				}
			}
		}
	}
}

namespace hooks {
	bool screen_location_switch_case(s32 n) {
		switch (n) {
			case 169:
			case 170:
			case 171:
			case 172:
			case 173:
			case 174:
				return true;
		}
		return false;
	}
}
