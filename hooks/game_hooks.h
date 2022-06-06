#pragma once

namespace hooks
{
	void __cdecl set_in_game_input_procs();
	bool game_on();
	bool game_end();
	void next_ui_frame();
	bool next_frame();
	void injectGameHooks();
}