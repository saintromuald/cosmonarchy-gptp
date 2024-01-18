#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

class Keyboard {
public:
	bool held(size_t i) { return is_held[i]; }
	u32 release_time(size_t i) { return released_for[i]; }
	u32 held_time(size_t i) { return held_for[i]; }
	bool pressed(size_t i) { return is_held[i] && !was_held[i]; }
	bool released(size_t i) { return !is_held[i] && was_held[i]; }

	void update() {
		bool is_active = GetActiveWindow() == *g_main_window;
		for (size_t i = 0; i < KEY_COUNT; i++) {
			was_held[i] = is_held[i];
			is_held[i] = is_active ? GetAsyncKeyState(i) : false;

			if (is_held[i] && was_held[i]) {
				released_for[i] = 0;
				held_for[i] += 1;
			} else {
				released_for[i] += 1;
				held_for[i] = 0;
			}
		}
	}

private:
	static constexpr size_t KEY_COUNT = 256;
	bool was_held[KEY_COUNT] = {};
	bool is_held[KEY_COUNT] = {};
	u32 released_for[KEY_COUNT] = {};
	u32 held_for[KEY_COUNT] = {};
};

extern Keyboard keyboard;

void init_hotkeys();
void update_hotkeys();

namespace hooks {
	bool screen_location_switch_case(s32 n);
	u8 game_input_user_key_press(DialogEvent* event);				// 004846B0

	void inject_hotkey_hooks();
}