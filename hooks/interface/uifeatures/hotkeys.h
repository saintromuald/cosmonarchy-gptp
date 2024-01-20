#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

struct KeyState {
	bool is_held;
	u32 last_pressed;
	u32 last_released;
	u32 last_held_time;
};

struct KeyEvent {
	char key;

	// Modifiers keys are commonly checked when processing a key event,
	// to keep the information about sequentiality we embed it here,
	// instead of checking if they are held this frame
	bool was_held;
	bool control_held;
	bool shift_held;
	bool alt_held;

	// JAI's using would be perfect here, since ideally we want to inherit all the vars instead of accessor methods,
	// but we also want to be able to say `ev.state = state`
	KeyState state;

	inline bool pressed() const { return !was_held && state.is_held; }
	inline bool held() const { return state.is_held; }
	inline bool released() const { return !state.is_held; }
	inline u32 held_time() const { return *g_elapsed_time_frames - state.last_pressed; }
	inline u32 release_time() const { return *g_elapsed_time_frames - state.last_released; }
	inline u32 last_held_time() const { return state.last_held_time; }
};

// Since std::move would clear capacity on Keyboard::poll() and that's means reallocating the buffer each frame
// this allows to iterate over the container and then clears it automatically (setting vec's size to 0, but not freeing the buffer)
template <typename Container>
class ClearSpan {
public:
	explicit ClearSpan(Container& container) : container(container) {}
	ClearSpan(const ClearSpan&) = delete;
	ClearSpan& operator=(const ClearSpan&) = delete;
	~ClearSpan() { container.clear(); }

	typename Container::iterator begin() { return container.begin(); }
	typename Container::iterator end() { return container.end(); }
	typename Container::const_iterator cbegin() const { return container.cbegin(); }
	typename Container::const_iterator cend() const { return container.cend(); }

private:
	Container& container;
};

class Keyboard {
public:
	// In general keys should be handled using keyboard events in handle_hotkeys event loop,
	// since checking if key is held on a given frame does not account for sequentiality of presses!
	// Exposing this only to BW's functionality that cannot be wired using the event loop.
	bool key_held_this_frame(char key) const { return key_states[key].is_held; }
	
	void add_key_event(int key, bool was_held, bool is_held) {
		if (key >= KEY_COUNT) return;

		KeyState& state = key_states[key];
		state.is_held = is_held;
		if (!was_held && is_held) {
			state.last_pressed = *g_elapsed_time_frames;
		}
		if (!is_held) {
			state.last_held_time = *g_elapsed_time_frames - state.last_pressed;
			state.last_released = *g_elapsed_time_frames;
		}

		KeyEvent ev = {};
		ev.key          = key;
		ev.was_held     = was_held;
		ev.control_held = key_states[VK_CONTROL].is_held;
		ev.shift_held   = key_states[VK_SHIFT].is_held;
		ev.alt_held     = key_states[VK_MENU].is_held;
		ev.state        = state;
		events.push_back(ev);
	}

	void update() {
		for (size_t i = 0; i < KEY_COUNT; i++) {
			KeyState& state = key_states[i];
			if (state.is_held) {
				// To release keys when we lose window focus
				bool is_pressed = GetKeyState(i) & (1 << 15);
				if (!is_pressed) {
					add_key_event(i, true, false);
				}
			}
		}
	}

	ClearSpan<std::vector<KeyEvent>> poll() {
		return ClearSpan<std::vector<KeyEvent>>(events);
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
