#include "game_hooks.h"
#include <BW/api.h>
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
			hooks::game_init();
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
			GPTP::logger.end_game();
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
		if (bw::is_game_paused())
		{
			__asm {
				POPAD
				MOV EAX, 1
				RETN
			}
		} else {
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
		strcat(buffer, g_config.default_map_directory.c_str());
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
		strcat(buffer, g_config.default_map_directory.c_str());
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	static std::filesystem::path get_replay_path() {
		return bw::get_starcraft_path() / "maps" / "replays";
	}

	void copy_last_replay(const std::string& dir, const std::string& name) {
		std::ifstream from(get_replay_path() / "LastReplay.rep", std::ios::binary);
		if (!from) {
			return;
		}

		std::filesystem::path to_path = get_replay_path() / "Autosave" / dir / name;
		if (std::filesystem::exists(to_path)) {
			return; // Do not overwrite existing files
		}

		std::error_code error;
		std::filesystem::create_directories(to_path.parent_path(), error);

		std::ofstream to(to_path, std::ios::binary);
		if (!to) {
			return;
		}

		to << from.rdbuf();
	}

	void replay_save() {
		if (!g_config.auto_save_replays) {
			return;
		}

		time_t now = time(0);
		tm* ltm = localtime(&now);

		char short_date[28] = {};
		strftime(short_date, 28, "%Y-%m-%d", ltm);

		char date[28] = {};
		strftime(date, 28, "%y-%m-%d_%H-%M", ltm);

		char name[28] = {};
		snprintf(name, 28, "%s_%s.rep", date, g_current_map_name);

		/*
		bool first = true;
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (g_player_table[i].type != PlayerType::Human && g_player_table[i].type != PlayerType::Computer) {
				continue;
			}
			if (globals->players[i].is_witness) {
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

		copy_last_replay(short_date, name);
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
			CMP bl, is_league // original code
			JMP jmp_address
		}
	}

	void __declspec(naked) initialize_provider_version_wrapper() { // 00472770
		static GameVersion* version;
		__asm {
			MOV version, ESI
			PUSHAD
		}
		hooks::initialize_provider_version(version);
		__asm {
			POPAD
			MOV EAX, 0
			RET
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

		jump_patch(initialize_provider_version_wrapper, 0x00472770, 1);
	}
}