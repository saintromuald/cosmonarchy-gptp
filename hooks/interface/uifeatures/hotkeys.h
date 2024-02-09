#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

struct KeyState {
	bool is_held;
	u64 last_pressed;
	u64 last_released;
	u64 last_held_time;
};

struct KeyEvent {
	u32 key;
	bool was_held;

	// Modifiers keys are commonly checked when processing a key event,
	// to keep the information about sequentiality we embed it here,
	// instead of checking if they are held this frame
	bool control_held;
	bool shift_held;
	bool alt_held;

	// JAI's using would be perfect here, since ideally we want to inherit all the vars instead of accessor methods,
	// but we also want to be able to say `ev.state = state`
	KeyState state;

	inline bool pressed() const { return !was_held && state.is_held; }
	inline bool held() const { return state.is_held; }
	inline bool released() const { return !state.is_held; }
	inline u64 held_time() const { return GetTickCount64() - state.last_pressed; }
	inline u64 release_time() const { return GetTickCount64() - state.last_released; }
	inline u64 last_held_time() const { return state.last_held_time; }
};

class Keyboard {
public:
	// In general keys should be handled using keyboard events in handle_hotkeys event loop,
	// since checking if key is held on a given frame does not account for sequentiality of presses!
	// Exposing this only to BW's functionality that cannot be wired using the event loop.
	bool key_held_this_frame(char key) const { return key < KEY_COUNT ? key_states[key].is_held : false; }
	
	bool key_event(u32 key, bool was_held, bool is_held) {
		if (key >= KEY_COUNT) return false;

		u64 now = GetTickCount64();
		KeyState& state = key_states[key];
		state.is_held = is_held;
		if (!was_held && is_held) {
			state.last_pressed = now;
		}
		if (!is_held) {
			state.last_held_time = now - state.last_pressed;
			state.last_released = now;
		}

		KeyEvent ev = {};
		ev.key          = key;
		ev.was_held     = was_held;
		ev.control_held = GetKeyState(VK_CONTROL) & (1 << 15);
		ev.shift_held   = GetKeyState(VK_SHIFT)   & (1 << 15);
		ev.alt_held     = GetKeyState(VK_MENU)    & (1 << 15);
		ev.state        = state;
		return handle_key_event(ev);
	}
	bool handle_key_event(const KeyEvent& ev);

	void update() {
		// TODO: maybe only do those release events on window focus loss
		for (size_t i = 0; i < KEY_COUNT; i++) {
			KeyState& state = key_states[i];
			if (state.is_held) {
				// To release keys when we lose window focus
				bool is_pressed = GetKeyState(i) & (1 << 15);
				if (!is_pressed) {
					key_event(i, true, false);
				}
			}
		}
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
