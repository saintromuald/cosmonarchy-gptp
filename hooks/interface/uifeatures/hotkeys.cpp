#include "hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>
#include "hooks/interface/buttonsets.h"
#include "aise_signal.h"

#define CameraCap 8
#define ControlGroupCap 17
#define ExtendedControlGroupKeys "RFVTGB"

struct Hotkeys {
	Point16 screens[CameraCap];
	ControlGroup control_groups[ControlGroupCap];
	ControlGroup* last_control_group_selected;
};

Keyboard keyboard;
static Hotkeys hotkeys;

static LRESULT CALLBACK keyboard_proc(int code, WPARAM wparam, LPARAM lparam) {
	u32 key = wparam;
	bool was_held = lparam & (1 << 30);
	bool is_held = !(lparam & (1 << 31));
	if (keyboard.key_event(key, was_held, is_held)) {
		return 1;
	}
	return CallNextHookEx(0, code, wparam, lparam);
}

void init_hotkeys() {
	static bool init_proc = false;
	if (!init_proc) {
		init_proc = true;
		SetWindowsHookA(WH_KEYBOARD, keyboard_proc);
	}

	keyboard = {};
	hotkeys = {};
	for (u32 i = 0; i < CameraCap; ++i) {
		hotkeys.screens[i].x = *g_move_to_x;
		hotkeys.screens[i].y = *g_move_to_y;
	}
}

static int control_group_index_from_key(u32 key) {
	if (key >= '0' && key <= '9') {
		return key - '0';
	}
	if (config.extended_control_groups) {
		const char* keys = "RFVTGB"; // NOTE: lower case letters do not work on some computers...
		for (u32 i = 0; i < 6; i++) {
			if (key == keys[i]) {
				return 10 + i;
			}
		}
	}
	if (config.tilde_control_group && key == 192) {
		return 16;
	}
	return -1;
}

static bool update_control_groups(const KeyEvent& ev) {
	s32 index = control_group_index_from_key(ev.key);
	if (index == -1) {
		return false;
	}

	ControlGroup& control_group = hotkeys.control_groups[index];
	bool center = false;
	bool just_pressed = false;
	if (ev.held()) {
		center = ev.held_time() >= 500;
		just_pressed = ev.pressed();
	}
	bool play_response = just_pressed;
	if (hotkeys.last_control_group_selected == &control_group) {
		if (ev.release_time() <= 500) {
			if (ev.pressed()) {
				center = true;
				play_response = false;
			}
		} else {
			hotkeys.last_control_group_selected = 0;
		}
	}

	if (ev.shift_held) {
		control_group.add_selection();
	} else if (ev.control_held || ev.alt_held) {
		control_group.clear();
		control_group.add_selection();
	} else {
		if (center) {
			control_group.center_camera();
		}
		if (just_pressed || center) {
			control_group.select(play_response);
			hotkeys.last_control_group_selected = &control_group;
		}
	}

	bool steal = false;
	if (config.default_to_steal_hotkeys) {
		steal = ev.control_held || ev.shift_held;
	} else {
		steal = ev.alt_held;
	}
	if (steal) {
		for (u32 i = 0; i < ControlGroupCap; i++) {
			if (&control_group == &hotkeys.control_groups[i]) {
				continue;
			}
			for (u32 j = 0; j < SelectionCap; j++) {
				Unit* unit = control_group.unit_index[j].get();
				if (unit) {
					hotkeys.control_groups[i].remove(UnitIndex::from(unit));
				}
			}
		}
	}
	return true;
}

bool Keyboard::handle_key_event(const KeyEvent& ev) {
	if (ev.key == VK_F4 && ev.alt_held) {
		return false;
	}
	if (aise_state.console_visible) {
		return false;
	}

	if (config.extended_camera_hotkeys) {
		if (ev.key >= VK_F1 && ev.key < VK_F1 + CameraCap) {
			Point16& screen = hotkeys.screens[ev.key - VK_F1];
			if (ev.control_held || ev.shift_held) {
				screen.x = *g_move_to_x;
				screen.y = *g_move_to_y;
			} else {
				helper::move_screen(screen);
			}
			return true;
		}
	}

	if (update_control_groups(ev)) {
		return true;
	}

	return false;
}

void update_hotkeys() {
	keyboard.update();
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

	// Hooked to add control group keyboard shortcut customization. -Pr0nogo
	u8 game_input_user_key_press(DialogEvent* event_arg) { // 004846B0
		u32 key_pressed = (u16)event_arg->virtual_key;
		if (key_pressed == VK_ESCAPE) {
			return helper::keypress_escape();
		} else {
			switch (key_pressed) {
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '0': {
					// Handled in update_control_groups() now
					return 0;
				}
			}
		}
	}
}
