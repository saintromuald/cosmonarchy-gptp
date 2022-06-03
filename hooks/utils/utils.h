#pragma once
#include "../../SCBW/structures.h"

namespace hooks
{
	void initialize_dialog(BinDlg* dialog, void* dlgFunc);	// 00419D20
	bool unit_is_standard_and_movable(Unit* unit);			// 0047B770
	Unit* get_active_player_first_selection();					// 0049A850

	void injectUtilsHooks();
}