#include "screen_hotkeys.h"
#include "SCBW/api.h"
#include <windows.h>

namespace
{
	s32 moveScreen(s32 x, s32 y);
	void centerviewUnitGroup(u8 a1);
	s32 selectUnitGroup(u8 a2);
}

namespace hooks
{
	void initializeScreenLocations()
	{
		for (Point16& point : screen_hotkeys)
		{
			point = {(u16)*MoveToX, (u16)*MoveToY};
		}
	}

	s32 saveScreenLocation(s32 id)
	{
		screen_hotkeys[id] = {(u16)*MoveToX, (u16)*MoveToY};
		return id;
	}

	s32 recallScreenLocation(s32 id)
	{
		Point16& point = screen_hotkeys[id];
		return moveScreen(point.x, point.y);
	}

	bool screenLocationSwitchCase(s32 n)
	{
		switch (n)
		{
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

	void handleScreenHotkeys()
	{ // a hacky workaround, checked every frame
		for (s32 i = 0; i < 8; i++)
		{
			s32 key = VK_F1 + i;
			if (GetKeyState(key) & 0x8000)
			{
				if (GetKeyState(VK_SHIFT) & 0x8000 || GetKeyState(VK_CONTROL) & 0x8000)
				{
					saveScreenLocation(i);
					break;
				}
				else
				{
					recallScreenLocation(i);
					break;
				}
			}
		}
	}

	Point16 screen_hotkeys[8] = {{0, 0}};
}

namespace
{
	const u32 moveScreen_function = 0x0049C440;
	s32 moveScreen(s32 x, s32 y)
	{
		static s32 result;
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


	const u32 centerviewUnitGroup_function = 0x004967E0;
	void centerviewUnitGroup(u8 a1)
	{
		_asm {
			PUSHAD
			PUSH a1
			CALL centerviewUnitGroup_function
			POPAD
		}
	}

	const u32 selectUnitGroup_function = 0x00496B40;
	s32 selectUnitGroup(u8 a2)
	{
		static s32 result;
		_asm {
			PUSHAD
			PUSH a2
			CALL selectUnitGroup_function
			MOV result, EAX
			POPAD
		}
		return result;
	}
}