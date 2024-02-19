#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

#define CameraCap 8
#define ControlGroupCap 17
#define ExtendedControlGroupKeys "RFVTGB"
#define KeyCap 256

struct KeyState {
	bool is_held;
	u64 last_pressed;
	u64 last_released;
	u64 last_held_time;
};

struct MouseState {
	s32 x;
	s32 y;
};

struct KeyEvent {
	u64 timestamp;
	u32 key;
	bool was_held;

	// Modifier keys are commonly checked when processing a key event,
	// to keep the information about sequentiality we embed it here,
	// instead of checking if they are held this frame
	bool control_held;
	bool shift_held;
	bool alt_held;

	// JAI's using would be perfect here, since ideally we want to inherit all the vars instead of accessor methods,
	// but we also want to be able to say `ev.state = state`
	KeyState state;

	// TODO: Consider splitting to seperate events or union?
	MouseState mouse_state;

	inline bool pressed() const { return !was_held && state.is_held; }
	inline bool held() const { return state.is_held; }
	inline bool released() const { return !state.is_held; }
	inline u64 held_time() const { return timestamp - state.last_pressed; }
	inline u64 release_time() const { return timestamp - state.last_released; }
	inline u64 last_held_time() const { return state.last_held_time; }
};

struct Hotkeys {
	Point16 screens[CameraCap];
	ControlGroup control_groups[ControlGroupCap];
	ControlGroup* last_control_group_selected;
	KeyState keys[KeyCap];
};

extern Hotkeys hotkeys;

void init_hotkeys();
void update_hotkeys();

// In general keys should be handled using keyboard events in handle_hotkeys event loop,
// since checking if key is held on a given frame does not account for sequentiality of presses!
// Exposing this only to BW's functionality that cannot be wired using the event loop.
bool key_held_this_frame(char key);

namespace hooks {
	bool screen_location_switch_case(s32 n);
	u8 game_input_user_key_press(DialogEvent* event);				// 004846B0

	void inject_hotkey_hooks();
}
