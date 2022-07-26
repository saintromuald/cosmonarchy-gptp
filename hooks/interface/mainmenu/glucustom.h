#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void display_establishing_shot();					// 004D9200, hooked by X405
	void cmdact_game_speed(BinDlg* control);			// 00481EF0

	void inject_glucustom_hooks();

	// DarkenedFantasies \\

	void sub_41D160();
	void sub_4EF600();
	void sub_4EF710();
	void loadMenu_gluScore_pIcons();
}