#include "game_hooks.h"
#include <SCBW/api.h>
#include <hook_tools.h>
#include <logger.h>
#include <config.h>
#include <trigger_system\trigger_system.h>
#include <player_color.h>

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
			GPTP::logger.start_game();
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

	void __declspec(naked) update_doodad_visibility_wrapper() { // 00487690
		static Thingy* thingy;
		static b32 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV thingy, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::update_doodad_visibility(thingy);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x04
		}
	}

	void __declspec(naked) game_recv_wrapper() { // 004865D0
		static char* cur_len;
		static s32 data_len;
		static bool as_replay;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV data_len, EAX
			MOV EAX, [EBP + 0x0C]
			MOV as_replay, AL
			POP EAX
			MOV cur_len, EAX
			PUSHAD
		}
		hooks::game_recv(cur_len, data_len, as_replay);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) sp_save_directory_wrapper() { // 004AE9E2
		static const u32 jmp_address = 0x004AEA15;
		static char* buffer;
		__asm {
			PUSH EAX
			LEA EAX, [EBP - 0x104]
			MOV buffer, EAX
			POP EAX
			PUSHAD
		}
		strcat(buffer, "\\");
		strcat(buffer, config.default_map_directory.c_str());
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) mp_save_directory_wrapper() { // 0044CCF7
		static const u32 jmp_address = 0x0044CD25;
		static char* buffer;
		__asm {
			MOV buffer, EDX
			PUSHAD
		}
		strcat(buffer, "\\");
		strcat(buffer, config.default_map_directory.c_str());
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	static char* get_replay_path(char* name) {
		char module[MAX_PATH] = {};
		if (GetModuleFileNameA(0, module, MAX_PATH) ) {
			char* c = strrchr(module, '\\');
			if (c) {
				*c = 0;
			}
		}
		static char path[MAX_PATH] = {};
		sprintf_s(path, "%s\\maps\\replays\\%s.rep", module, name);
		return path;
	}

	void copy_last_replay(char* name) {
		FILE* from = fopen(get_replay_path("LastReplay"), "rb");
		if (!from) {
			return;
		}

		FILE* to = fopen(get_replay_path(name), "wb");
		if (!to) {
			fclose(from);
			return;
		}

		char buffer[4096] = {};
		while (u32 bytes_read = fread(buffer, 1, 4096, from)) {
			fwrite(buffer, 1, bytes_read, to);
		}

		fclose(from);
		fclose(to);
	}

	void replay_save() {
		char date[28] = {};
		time_t now = time(0);
		tm* ltm = localtime(&now);
		strftime(date, 28, "%y-%m-%d_%H-%M", ltm);

		char name[28] = {};
		snprintf(name, 28, "%s_%s", date, g_current_map_name);

		/*
		bool first = true;
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (g_player_table[i].type != PlayerType::Human && g_player_table[i].type != PlayerType::Computer) {
				continue;
			}
			if (global_vars.players[i].is_witness) {
				continue;
			}
			if (first) {
				strcat_s(name, ", ");
			} else {
				strcat_s(name, " - ");
			}
			strcat_s(name, player_data.full_player_names[i].data);
			first = false;
		}
		*/

		if (!file_exists(get_replay_path(name))) {
			copy_last_replay(name);
		}
	}

	void __declspec(naked) replay_save_wrapper() { // 004EEBEA
		static const u32 jmp_address = 0x004EEBF0;
		__asm {
			PUSHAD
		}
		replay_save();
		static u8 is_league = *g_is_league_game;
		__asm {
			POPAD
			CMP bl, is_league
			JMP jmp_address
		}
	}

//-------- Hook injector --------//

namespace hooks {
	void inject_game_hooks() {
		jump_patch(on_game_start,						Hook_OnGameStart);
		jump_patch(set_ingame_input_procs,				0x00484CC0, 0);
		jump_patch(update_minimap_colors,				0x004A3A00, 0);
		jump_patch(update_doodad_visibility_wrapper,	0x00487690, 1);
		jump_patch(game_recv_wrapper,					0x004865D0, 0);

		jump_patch(ui_button_click_wrapper,				0x004190ED, 2);

		call_patch(on_game_end,							0x004EE983, 0);
		call_patch(next_frame_hook,						0x004D974E, 0);
		call_patch(next_ui_frame_hook,					0x004DE433, 0);

		jump_patch(sp_save_directory_wrapper,			0x004AE9E2, 0);
		jump_patch(mp_save_directory_wrapper,			0x0044CCF7, 1);
		jump_patch(replay_save_wrapper,					0x004EEBEA, 1);
	}
}