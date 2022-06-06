#pragma once

namespace hooks
{
	extern bool quickcasting_signal;
	extern u32 quickcasting_command_card;

	u32 __cdecl set_ingame_input_procs(); // 00484CC0
	bool game_on();
	bool game_end();
	void next_ui_frame();
	bool next_frame();
	void inject_game_hooks();
}