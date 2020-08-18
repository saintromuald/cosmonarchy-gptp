#pragma once
#include "../SCBW/scbwdata.h"

namespace hooks {
	void initializeScreenLocations();
	int saveScreenLocation(int id);
	int recallScreenLocation(int id);
	int moveScreen(int x, int y);
	bool screenLocationSwitchCase(int n);

	void injectScreenHotkeysHooks();
	extern Point16 screen_hotkeys[8];
}