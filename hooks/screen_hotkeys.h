#pragma once
#include "../SCBW/scbwdata.h"
#include "../logger.h"

namespace hooks {
	void initializeScreenLocations();
	int saveScreenLocation(int id);
	int recallScreenLocation(int id);
	bool screenLocationSwitchCase(int n);

	void handle_screenHotkeys();

	void injectScreenHotkeysHooks();
	extern Point16 screen_hotkeys[8];
}