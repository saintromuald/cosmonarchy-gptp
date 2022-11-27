#pragma once

namespace hooks {
	extern u8 quickcasting_timeout;
	extern u8 quickbuilding_timeout;
	extern bool quickcasting_signal;
	extern u32 quickcasting_command_card;
	extern Unit* transport_ui_unit;

	void __cdecl update_minimap_colors(); // 004A3A00

	void init_unit(Unit* unit);
	void tick_unit(Unit* unit, bool unit_is_hidden);

	void ui_button_click(BinDlg* dialog);
	u32 __cdecl set_ingame_input_procs(); // 00484CC0
	bool game_on();
	bool game_end();
	void next_ui_frame();
	bool next_frame();
	void inject_game_hooks();
}