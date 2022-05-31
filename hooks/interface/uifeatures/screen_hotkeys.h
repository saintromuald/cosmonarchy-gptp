#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

namespace hooks
{
	void initalize_screen_locations();
	s32 saveScreenLocation(s32 id);
	s32 recallScreenLocation(s32 id);
	bool screenLocationSwitchCase(s32 n);

	void handle_screen_hotkeys();

	void injectScreenHotkeysHooks();
	extern Point16 screen_hotkeys[8];
}