#include "hotkeys.h"
#include "BW/api.h"
#include "hooks/interface/buttonsets.h"
#include "hooks/interface/hud/hud_manager.h"
#include "aise_signal.h"

#include <windows.h>

Hotkeys hotkeys;

static KeyEvent key_event(u32 key, bool was_held, bool is_held) {
	u64 now = GetTickCount64();
	KeyState& state = hotkeys.keys[key];
	state.is_held = is_held;
	if (!was_held && is_held) {
		state.last_pressed = now;
	}
	if (!is_held) {
		state.last_held_time = now - state.last_pressed;
		state.last_released = now;
	}

	KeyEvent ev = {};
	ev.timestamp    = now;
	ev.key          = key;
	ev.was_held     = was_held;
	ev.control_held = GetKeyState(VK_CONTROL) & (1 << 15);
	ev.shift_held   = GetKeyState(VK_SHIFT)   & (1 << 15);
	ev.alt_held     = GetKeyState(VK_MENU)    & (1 << 15);
	ev.state        = state;
	return ev;
}

static int control_group_index_from_key(u32 key) {
	if (key >= '0' && key <= '9') {
		return key - '0';
	}
	if (g_config.extended_control_groups) {
		const char* keys = "RFVTGB"; // NOTE: lower case letters do not work on some computers...
		for (u32 i = 0; i < 6; i++) {
			if (key == keys[i]) {
				return 10 + i;
			}
		}
	}
	if (g_config.tilde_control_group && key == 192) {
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
	if (g_config.default_to_steal_hotkeys) {
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
					hotkeys.control_groups[i].remove(unit);
				}
			}
		}
	}
	return true;
}

static bool handle_key_event(const KeyEvent& ev) {
	// Ignore when in UI
	if (!hotkeys.game_started || g_popup_dialog || *g_chat_message_type || g_aise_state.console_visible) {
		return false;
	}
	// Ignore Alt+F4
	if (ev.alt_held && ev.key == VK_F4) {
		return false;
	}
	// Ignore Ctrl+Q
	if (ev.control_held && ev.key == 'Q') {
		return true;
	}
	// Toggle hud visibility
	if (ev.key == VK_F11 && ev.pressed() && g_hud_manager && g_hud_manager->hotkey_enabled()) {
		if (g_hud_manager->is_hidden()) {
			g_hud_manager->show();
		} else {
			g_hud_manager->hide();
		}
		return true;
	}
	// Specifically ignore 1-6 for faction selection for non-grid hotkeys
	if (!g_config.grid_hotkeys && buttonset_id_is_faction_selection(g_current_buttonset_id)) {
		if (ev.key >= '1' && ev.key <= '6' && !ev.control_held && !ev.shift_held && !ev.alt_held) {
			return false;
		}
	}

	// Trigger Buttons
	if (buttons_handle_input(ev)) {
		return true;
	}

	// Cameras
	if (g_config.extended_camera_hotkeys) {
		if (ev.key >= VK_F1 && ev.key < VK_F1 + CameraCap) {
			v2u16& screen = hotkeys.screens[ev.key - VK_F1];
			if (ev.control_held || ev.shift_held) {
				screen.x = g_move_to_x;
				screen.y = g_move_to_y;
			} else {
				helper::move_screen(screen);
			}
			return true;
		}
	}

	// Control Groups
	if (update_control_groups(ev)) {
		return true;
	}

	return false;
}
static LRESULT CALLBACK keyboard_proc(int code, WPARAM wparam, LPARAM lparam) {
	u32 key = wparam;
	bool was_held = lparam & (1 << 30);
	bool is_held = !(lparam & (1 << 31));
	if (key < KeyCap) {
		if (handle_key_event(key_event(key, was_held, is_held))) {
			return 1;
		}
	}
	return CallNextHookEx(0, code, wparam, lparam);
}

static WNDPROC original_window_proc;
static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
		case WM_LBUTTONDOWN: {
			if (handle_key_event(key_event(VK_LBUTTON, false, true))) {
				return 0;
			}
		} break;
		case WM_LBUTTONUP: {
			if (handle_key_event(key_event(VK_LBUTTON, true, false))) {
				return 0;
			}
		} break;
		case WM_MBUTTONDOWN: {
			if (handle_key_event(key_event(VK_MBUTTON, false, true))) {
				return 0;
			}
		} break;
		case WM_MBUTTONUP: {
			if (handle_key_event(key_event(VK_MBUTTON, true, false))) {
				return 0;
			}
		} break;
		case WM_RBUTTONDOWN: {
			if (handle_key_event(key_event(VK_RBUTTON, false, true))) {
				return 0;
			}
		} break;
		case WM_RBUTTONUP: {
			if (handle_key_event(key_event(VK_RBUTTON, true, false))) {
				return 0;
			}
		} break;
	}
	return original_window_proc(hwnd, message, wparam, lparam);
}

void init_hotkeys() {
	static bool init_proc = false;
	if (!init_proc) {
		init_proc = true;
		SetWindowsHookA(WH_KEYBOARD, keyboard_proc);
		// original_window_proc = (WNDPROC)SetWindowLongPtrA(*g_main_window, GWLP_WNDPROC, (LONG_PTR)window_proc);
	}

	hotkeys = {};
	hotkeys.game_started = true;
	for (u32 i = 0; i < CameraCap; ++i) {
		hotkeys.screens[i].x = g_move_to_x;
		hotkeys.screens[i].y = g_move_to_y;
	}
}


void update_hotkeys() {
	// TODO: maybe only do those release events on window focus loss
	for (size_t i = 0; i < KeyCap; i++) {
		KeyState& state = hotkeys.keys[i];
		if (state.is_held) {
			// To release keys when we lose window focus
			bool is_pressed = GetKeyState(i) & (1 << 15);
			if (!is_pressed) {
				handle_key_event(key_event(i, true, false));
			}
		}
	}
}

void inherit_selection(Unit* parent, Unit* child) {
	// TODO: also inherit selection
	inherit_control_groups(parent, child);
}

void inherit_control_groups(Unit* parent, Unit* child) {
	for (ControlGroup& control_group : hotkeys.control_groups) {
		if (control_group.has(parent)) {
			control_group.add(child);
		}
	}
}

bool key_held_this_frame(char key) { return key < KeyCap ? hotkeys.keys[key].is_held : false; }

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
