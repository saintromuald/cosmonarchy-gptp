#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

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
