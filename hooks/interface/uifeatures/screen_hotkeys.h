#pragma once
#include "SCBW/scbwdata.h"
#include "logger.h"

namespace hooks {
	void initializeScreenLocations();
	s32 saveScreenLocation(s32 id);
	s32 recallScreenLocation(s32 id);
	bool screenLocationSwitchCase(s32 n);

	void handleScreenHotkeys();

	void injectScreenHotkeysHooks();
	extern Point16 screen_hotkeys[8];
}