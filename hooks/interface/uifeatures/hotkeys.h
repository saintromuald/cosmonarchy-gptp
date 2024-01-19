#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

struct KeyState {
	bool was_held;
	bool is_held;
	u32 held_time;
	u32 release_time;
	u32 last_held_time;
};

struct KeyEvent {
	char key;
	KeyState state;

	inline bool pressed() const { return !state.was_held && state.is_held; }
	inline bool held() const { return state.is_held; }
	inline bool released() const { return !state.is_held; }
	inline u32 held_time() const { return state.held_time; }
	inline u32 release_time() const { return state.release_time; }
	inline u32 last_held_time() const { return state.last_held_time; }
};

class Keyboard {
public:
	bool held(size_t i) { return key_states[i].is_held; } // Use for modifier keys only!
	
	void add_key_event(char key, bool was_held, bool is_held) {
		if (key >= KEY_COUNT) return;

		KeyState& state = key_states[key];
		state.was_held = was_held;
		state.is_held = is_held;
		if (!is_held) {
			state.last_held_time = state.held_time;
			state.held_time = 0;
			state.release_time = 0;
		}

		KeyEvent ev = {};
		ev.key = key;
		ev.state = state;
		events.push_back(ev);
	}

	void update() {
		for (size_t i = 0; i < KEY_COUNT; i++) {
			KeyState& state = key_states[i];
			if (state.is_held) {
				state.held_time += 1;
				state.release_time += 1;

				// To release keys when we lose window focus
				bool is_pressed = GetKeyState(i) & (1 << 15);
				if (!is_pressed) {
					add_key_event(i, true, false);
				}
			} else {
				state.was_held = false;
			}
		}
	}

	std::vector<KeyEvent> poll() {
		return std::move(events);
	}

private:
	static constexpr size_t KEY_COUNT = 256;
	KeyState key_states[KEY_COUNT] = {};
	std::vector<KeyEvent> events;
};

extern Keyboard keyboard;

void init_hotkeys();
void update_hotkeys();

namespace hooks {
	bool screen_location_switch_case(s32 n);
	u8 game_input_user_key_press(DialogEvent* event);				// 004846B0

	void inject_hotkey_hooks();
}
