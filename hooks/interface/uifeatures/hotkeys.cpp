#include "hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>
#include "hooks/interface/buttonsets.h"

#define CameraCap 8
#define StandardControlGroups 10
#define ExtendedControlGroups 16

static const char control_group_keys[] = "0123456789rfvtgb";
static_assert(sizeof(control_group_keys) == ExtendedControlGroups + 1); // null term

struct Hotkeys {
	Point16 screens[CameraCap];
	ControlGroup control_groups[ExtendedControlGroups];
	ControlGroup* last_control_group_selected;
};

Keyboard keyboard;
static Hotkeys hotkeys;

void init_hotkeys() {
	keyboard = {};
	hotkeys = {};
	for (u32 i = 0; i < CameraCap; ++i) {
		hotkeys.screens[i].x = *g_move_to_x;
		hotkeys.screens[i].y = *g_move_to_y;
	}
}

static void update_control_groups() {
	u32 count = config.extended_control_groups ? ExtendedControlGroups : StandardControlGroups;
	ControlGroup* control_group = 0;
	bool center = false;
	for (u32 i = 0; i < count; i++) {
		char key = control_group_keys[i];
		if (keyboard.held(key)) {
			control_group = &hotkeys.control_groups[i];
			center = keyboard.held_time(key) >= 8;
			if (keyboard.pressed(key) && keyboard.release_time(key) <= 6 &&
					hotkeys.last_control_group_selected == control_group) {
				center = true;
			}
		}
	}

	if (control_group) {
		if (keyboard.held(VK_SHIFT)) {
			control_group->add_selection();
		} else if (keyboard.held(VK_CONTROL) || keyboard.held(VK_MENU)) {
			control_group->clear();
			control_group->add_selection();
		} else if (center) {
			control_group->center_camera();
		} else {
			control_group->select();
			hotkeys.last_control_group_selected = control_group;
		}

		if (keyboard.held(VK_MENU)) {
			for (u32 i = 0; i < ExtendedControlGroups; i++) {
				if (control_group == &hotkeys.control_groups[i]) {
					continue;
				}
				for (u32 j = 0; j < SelectionCap; j++) {
					Unit* unit = control_group->unit_index[j].get();
					if (unit) {
						hotkeys.control_groups[i].remove(UnitIndex::from(unit));
					}
				}
			}
		}
	}
}

void update_hotkeys() {
	keyboard.update();

	if (config.extended_camera_hotkeys) {
		for (s32 i = 0; i < CameraCap; i++) {
			if (keyboard.held(VK_F1 + i)) {
				if (keyboard.held(VK_CONTROL) || keyboard.held(VK_SHIFT)) {
					hotkeys.screens[i].x = *g_move_to_x;
					hotkeys.screens[i].y = *g_move_to_y;
				} else {
					helper::move_screen(hotkeys.screens[i]);
				}
			}
		}
	}

	update_control_groups();
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
