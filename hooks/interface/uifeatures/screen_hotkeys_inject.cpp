#include "screen_hotkeys.h"
#include "hook_tools.h"
#include "config.h"

namespace
{
	void _declspec(naked) saveScreenLocation_Wrapper()
	{
		static s32 id;
		static s32 result;
		_asm {
			MOV id, EAX
			PUSHAD
		}
		result = hooks::saveScreenLocation(id);
		_asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void _declspec(naked) recallScreenLocation_Wrapper()
	{
		static s32 id;
		static s32 result;
		_asm {
			MOV id, EAX
			PUSHAD
		}
		result = hooks::recallScreenLocation(id);
		_asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	const u32 helpMenuCall_jmp = 0x0048478A;
	void _declspec(naked) helpMenuCall_Wrapper()
	{
		_asm {
			JMP helpMenuCall_jmp
		}
	}

	const u32 screenLocationSwitchCase_jmp = 0x00484796;
	const u32 loc_484B24 = 0x00484B24;
	void _declspec(naked) screenLocationSwitchCase_Wrapper()
	{
		static s32 v1;
		static bool result;
		_asm {
			MOV v1, EDI
			PUSHAD
		}
		if (!(*bw_is_game_paused))
		{
			result = hooks::screenLocationSwitchCase(v1 + 0x642D);
			if (!result)
			{
				_asm { // continue executing switch case if case wasn't found
					POPAD
					JMP screenLocationSwitchCase_jmp
				}
			}
		}
		_asm { // return
			POPAD
			JMP loc_484B24
		}
	}
}

namespace hooks
{

	void injectScreenHotkeysHooks()
	{
		if (config.extended_camera_hotkeys)
		{
			jump_patch(saveScreenLocation_Wrapper,			0x00484360, 0);
			jump_patch(recallScreenLocation_Wrapper,		0x00484500, 1);
			jump_patch(helpMenuCall_Wrapper,				0x00484770, 0);
			jump_patch(screenLocationSwitchCase_Wrapper,	0x0048478A, 1);
		}
	}
}