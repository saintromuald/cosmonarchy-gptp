#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	s32 init_game();									// 004EE110
	s32 start_game(void* lobby_state, s32 player);		// 00472060
	Player* init_singleplayer_custom(BinDlg* dialog);	// 004AD850

	void inject_lobby_hooks();
}