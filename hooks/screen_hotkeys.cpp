#include "screen_hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>

namespace {
	int moveScreen(int x, int y);
	u8 selectUnitGroup(u8 n);
	u8 keyPress_Escape();
	s32 GetKeyModifierFlags();
}

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

	u8 input_Game_UserKeyPress(dlgEvent* dlg) {
		u32 v1; // ebx
		u8 result; // al

		v1 = (unsigned __int16)dlg->wVirtKey;
		scbw::printFormattedText("%i", v1);
		if (v1 >= '0' && v1 <= '9')
			result = selectUnitGroup(v1 - '0');
		if (v1 == VK_ESCAPE)
			result = keyPress_Escape();
		return result;
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

	const u32 selectUnitGroup_function = 0x00496B40;
	u8 selectUnitGroup(u8 n) {
		static int result;
		_asm {
			PUSHAD
			PUSH n
			CALL selectUnitGroup_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 keyPress_Escape_function = 0x00489DA0;
	u8 keyPress_Escape() {
		static int result;
		_asm {
			PUSHAD
			CALL keyPress_Escape_function
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const u32 GetKeyModifierFlags_function = 0x004D1810;
	s32 GetKeyModifierFlags() {
		static int result;
		_asm {
			PUSHAD
			CALL GetKeyModifierFlags_function
			MOV result, EAX
			POPAD
		}
		return result;
	}
}