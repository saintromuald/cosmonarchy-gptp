#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

void init_screen_locations();
void handle_screen_hotkeys();

#define CameraCap 8
extern Point16 screen_hotkeys[CameraCap];

namespace hooks {
	s32 save_screen_location(s32 id);
	s32 recall_screen_location(s32 id);
	bool screen_location_switch_case(s32 n);

	void inject_screen_hotkey_hooks();
}