#pragma once

struct GameVersion;

namespace hooks {
	extern u8 quickcasting_timeout;
	extern u8 quickbuilding_timeout;
	extern bool quickcasting_signal;
	extern u32 quickcasting_command_card;
	extern Unit* transport_ui_unit;

	void __cdecl update_minimap_colors();							// 004A3A00
	b32 update_doodad_visibility(Thingy* thingy);					// 00487690
	void game_recv(char* buffer, s32 size, bool as_replay);			// 004865D0

	void init_unit(Unit* unit);
	void tick_unit(Unit* unit, bool unit_is_hidden);

	void ui_button_click(BinDlg* dialog);
	u32 __cdecl set_ingame_input_procs();							// 00484CC0
	bool game_on();
	bool game_end();
	void next_ui_frame();
	bool next_frame();
	void inject_game_hooks();

	void initialize_provider_version(GameVersion* version);			// 00472770
}