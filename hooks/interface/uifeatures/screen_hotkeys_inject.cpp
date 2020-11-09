#include "screen_hotkeys.h"
#include "hook_tools.h"
#include "config.h"

namespace {
	void _declspec(naked) saveScreenLocation_Wrapper() {
		static int id;
		static int result;
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

	void _declspec(naked) recallScreenLocation_Wrapper() {
		static int id;
		static int result;
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
	void _declspec(naked) helpMenuCall_Wrapper() {
		_asm {
			JMP helpMenuCall_jmp
		}
	}

	const u32 screenLocationSwitchCase_jmp = 0x00484796;
	const u32 loc_484B24 = 0x00484B24;
	void _declspec(naked) screenLocationSwitchCase_Wrapper() {
		static int v1;
		static bool result;
		_asm {
			MOV v1, EDI
			PUSHAD
		}
		if (!(*IS_GAME_PAUSED)) {
			result = hooks::screenLocationSwitchCase(v1 + 0x642D);
			if (!result) {
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

namespace hooks {

	void injectScreenHotkeysHooks() {
		if (config.extended_camera_hotkeys) {
			jmpPatch(saveScreenLocation_Wrapper, 0x00484360, 0);
			jmpPatch(recallScreenLocation_Wrapper, 0x00484500, 1);
			jmpPatch(helpMenuCall_Wrapper, 0x00484770, 0);
			jmpPatch(screenLocationSwitchCase_Wrapper, 0x0048478A, 1);
		}
	}
}