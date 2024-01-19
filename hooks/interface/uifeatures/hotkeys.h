#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

struct Key {
	bool was_held;
	bool is_held;
	u32 released_for;
	u32 held_for;
	u32 last_held_for;

	void update(bool key_held, bool window_active) {
		was_held = is_held;
		is_held = window_active ? key_held : false;

		if (is_held && was_held) {
			released_for = 0;
			held_for += 1;
		} else {
			if (was_held) {
				last_held_for = held_for;
			}
			released_for += 1;
			held_for = 0;
		}
	}
};

class Keyboard {
public:
	bool held(size_t i) { return keys[i].is_held; }
	u32 release_time(size_t i) { return keys[i].released_for; }
	u32 held_time(size_t i) { return keys[i].held_for; }
	u32 last_held_time(size_t i) { return keys[i].last_held_for; }
	bool pressed(size_t i) { return keys[i].is_held && !keys[i].was_held; }
	bool released(size_t i) { return !keys[i].is_held && keys[i].was_held; }

	void update() {
		bool window_active = GetActiveWindow() == *g_main_window;
		for (size_t i = 0; i < KEY_COUNT; i++) {
			bool key_held = GetAsyncKeyState(i) & (1 << 15);
			keys[i].update(key_held, window_active);
		}
	}

private:
	static constexpr size_t KEY_COUNT = 256;
	Key keys[KEY_COUNT] = {};
};

extern Keyboard keyboard;

void init_hotkeys();
void update_hotkeys();

namespace hooks {
	bool screen_location_switch_case(s32 n);
	u8 game_input_user_key_press(DialogEvent* event);				// 004846B0

	void inject_hotkey_hooks();
}