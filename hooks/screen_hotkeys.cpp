#include "screen_hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>

namespace {
	int moveScreen(int x, int y);
}

namespace hooks {
	void initializeScreenLocations() {
		for (Point16& point : screen_hotkeys) {
			point = {(u16)*MoveToX, (u16)*MoveToY};
		}
	}

	int saveScreenLocation(int id) {
		screen_hotkeys[id] = {(u16)*MoveToX, (u16)*MoveToY};
		return id;
	}

	int recallScreenLocation(int id) {
		Point16& point = screen_hotkeys[id];
		return moveScreen(point.x, point.y);
	}

	bool screenLocationSwitchCase(int n) {
		switch (n) {
		case 169:
			//saveScreenLocation(0);
			break;
		case 170:
			//saveScreenLocation(1);
			break;
		case 171:
			//saveScreenLocation(2);
			break;
		case 172:
			//recallScreenLocation(0);
			break;
		case 173:
			//recallScreenLocation(1);
			break;
		case 174:
			//recallScreenLocation(2);
			break;
		default:
			return false;
		}
		return true;
	}

	void handle_screenHotkeys() { // a hacky workaround, checked every frame
		for (int i = 0; i < 8; i++) {
			int key = VK_F1 + i;
			if (GetKeyState(key) & 0x8000) {
				if (GetKeyState(VK_SHIFT) & 0x8000 || GetKeyState(VK_CONTROL) & 0x8000) {
					saveScreenLocation(i);
					break;
				} else {
					recallScreenLocation(i);
					break;
				}
			}
		}
		
	}

	Point16 screen_hotkeys[8] = { {0, 0} };
}

namespace {
	const u32 moveScreen_function = 0x0049C440;
	int moveScreen(int x, int y) {
		static int result;
		_asm {
			PUSHAD
			MOV EAX, x
			MOV ECX, y
			CALL moveScreen_function
			MOV result, EAX
			POPAD
		}
		return result;
	}
}