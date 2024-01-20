#include "hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>
#include "hooks/interface/buttonsets.h"

#define CameraCap 8
#define StandardControlGroups 10
#define ExtendedControlGroups 16

static const char control_group_keys[] = "0123456789RFVTGB"; // NOTE: lower case letters do not work on some computers...
static_assert(sizeof(control_group_keys) == ExtendedControlGroups + 1); // null term

struct Hotkeys {
	Point16 screens[CameraCap];
	ControlGroup control_groups[ExtendedControlGroups];
	ControlGroup* last_control_group_selected;
};

Keyboard keyboard;
static Hotkeys hotkeys;

static LRESULT CALLBACK keyboard_proc(int code, WPARAM wparam, LPARAM lparam) {
	char key = wparam;
	bool was_held = lparam & (1 << 30);
	bool is_held = !(lparam & (1 << 31));
	keyboard.add_key_event(key, was_held, is_held);
	return 0;
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

static void update_control_groups(const KeyEvent& ev) {
	u32 count = config.extended_control_groups ? ExtendedControlGroups : StandardControlGroups;
	s32 index = -1;
	for (u32 i = 0; i < count; i++) {
		if (ev.key == control_group_keys[i]) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		return;
	}

	ControlGroup& control_group = hotkeys.control_groups[index];
	bool center = false;
	bool just_pressed = false;
	if (ev.held()) {
		center = ev.held_time() >= 12;
		just_pressed = ev.pressed();
	}
	if (hotkeys.last_control_group_selected == &control_group) {
		if (ev.release_time() <= 12) {
			if (ev.pressed()) {
				center = true;
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
			control_group.select();
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
		for (u32 i = 0; i < ExtendedControlGroups; i++) {
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
}

void update_hotkeys() {
	keyboard.update();

	for (const KeyEvent& ev : keyboard.poll()) {
		if (config.extended_camera_hotkeys) {
			if (ev.key == VK_F4 && ev.alt_held) {

			}
			if (ev.key >= VK_F1 && ev.key <= VK_F12) {
				Point16& screen = hotkeys.screens[ev.key - VK_F1];
				if (ev.control_held || ev.shift_held) {
					screen.x = *g_move_to_x;
					screen.y = *g_move_to_y;
				} else {
					helper::move_screen(screen);
				}
			}
		}
		
		update_control_groups(ev);
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
