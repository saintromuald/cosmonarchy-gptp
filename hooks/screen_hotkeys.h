#pragma once
#include "../SCBW/scbwdata.h"

namespace hooks {
	void initializeScreenLocations();
	int saveScreenLocation(int id);
	int recallScreenLocation(int id);
	bool screenLocationSwitchCase(int n);
	u8 input_Game_UserKeyPress(dlgEvent* dlg);

	void injectScreenHotkeysHooks();
	extern Point16 screen_hotkeys[8];
}