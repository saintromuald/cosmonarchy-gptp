#include "game_hooks.h"
#include <SCBW/api.h>
#include <hook_tools.h>
#include <logger.h>

bool isGameOn = false;

/******** Game Start ********/

	const u32 Hook_OnGameStart = 0x004C9A51;
	const u32 Hook_OnGameStartBack = Hook_OnGameStart + 5;
	const u32 Func_CompileSCode = 0x00417C20;
	void __declspec(naked) on_game_start()
	{ // From BWAPI by Kovarex, slightly modified
		__asm PUSHAD;
		{
			isGameOn = true;
			hooks::game_on();
			GPTP::logger.startGame();
		}
		__asm {
			POPAD
			CALL Func_CompileSCode
			JMP Hook_OnGameStartBack
		}
	}

	/******** Game End ********/

	const u32 Func_DeleteSCode = 0x00416D90;
	void __declspec(naked) on_game_end()
	{ // From BWAPI by Kovarex, slightly modified
		__asm PUSHAD;
		{
			isGameOn = false;
			hooks::game_end();
			GPTP::logger.endGame();
		}
		__asm {
			POPAD
			CALL Func_DeleteSCode
			RETN
		}
	}

	/******** NEXT FRAME HOOK ********/

	void __declspec(naked) next_frame_hook()
	{ // From BWAPI by Kovarex, slightly modified
		__asm {
			PUSHAD
			MOV EBP, ESP
		}
		{
			hooks::next_frame();
		}

		if (scbw::is_game_paused())
		{
			__asm {
				POPAD
				MOV EAX, 1
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				MOV EAX, 0
				RETN
			}
		}
	}

	void __declspec(naked) next_ui_frame_hook() {
		__asm {
			PUSHAD
			MOV EBP, ESP
		}
		hooks::next_ui_frame();
		__asm {
			POPAD
			MOV EDI, 4 // original line
			RETN
		}
	}

	void __declspec(naked) ui_button_click_wrapper() {
		static const u32 jmp_address = 0x004190F4;
		static BinDlg* dialog;
		__asm {
			MOV dialog, ESI
			PUSHAD
		}
		hooks::ui_button_click(dialog);
		__asm {
			POPAD
			MOV EDX, EBX
			MOV ECX, ESI
			CALL dword ptr [ESI + 0x2A]
			JMP jmp_address
		}
	}

//-------- Hook injector --------//

namespace hooks {
	void inject_game_hooks() {
		jump_patch(on_game_start,			Hook_OnGameStart);
		jump_patch(set_ingame_input_procs,	0x00484CC0, 0);

		jump_patch(ui_button_click_wrapper, 0x004190ED, 2);

		call_patch(on_game_end,				0x004EE983, 0);
		call_patch(next_frame_hook,			0x004D974E, 0);
		call_patch(next_ui_frame_hook,		0x004DE433, 0);
	}
}