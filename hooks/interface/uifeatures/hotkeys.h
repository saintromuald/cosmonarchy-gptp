#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

class Keyboard {
public:
	bool held(size_t i) { return is_held[i]; }
	bool pressed(size_t i) { return is_held[i] && !was_held[i]; }
	bool released(size_t i) { return !is_held[i] && was_held[i]; }

	void update() {
		bool is_active = GetActiveWindow() == *g_main_window;
		for (size_t i = 0; i < key_count; i++) {
			was_held[i] = is_held[i];
			is_held[i] = is_active ? GetAsyncKeyState(i) : false;
		}
	}

private:
	static constexpr size_t key_count = 256;
	bool was_held[key_count] = {};
	bool is_held[key_count] = {};
};

extern Keyboard keyboard;

void init_hotkeys();
void update_hotkeys();

namespace hooks {
	bool screen_location_switch_case(s32 n);

	void inject_hotkey_hooks();
}