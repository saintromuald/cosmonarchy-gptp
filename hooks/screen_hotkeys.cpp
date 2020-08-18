#include "screen_hotkeys.h"
#include "SCBW/api.h"

namespace hooks {
	void initializeScreenLocations() {
		for (Point16& point : screen_hotkeys) {
			point = {(u16)*MoveToX, (u16)*MoveToY};
		}
	}

	int saveScreenLocation(int id) {
		screen_hotkeys[id] = {(u16)*MoveToX, (u16)*MoveToY};
		//scbw::printFormattedText("Saving id %i", id);
		return id;
	}

	int recallScreenLocation(int id) {
		Point16& point = screen_hotkeys[id];
		//scbw::printFormattedText("Recalling id %i", id);
		return moveScreen(point.x, point.y);
	}

	bool screenLocationSwitchCase(int n) {
		//scbw::printFormattedText("%i", n);
		switch (n) {
		case 169:
			saveScreenLocation(0);
			break;
		case 170:
			saveScreenLocation(1);
			break;
		case 171:
			saveScreenLocation(2);
			break;
		case 172:
			recallScreenLocation(0);
			break;
		case 173:
			recallScreenLocation(1);
			break;
		case 174:
			recallScreenLocation(2);
			break;
		default:
			return false;
		}
		return true;
	}

	Point16 screen_hotkeys[8] = { {0, 0} };
}