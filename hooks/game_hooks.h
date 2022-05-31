#pragma once

namespace hooks
{
	bool game_on();
	bool game_end();
	void next_ui_frame();
	bool next_frame();
	void injectGameHooks();
}