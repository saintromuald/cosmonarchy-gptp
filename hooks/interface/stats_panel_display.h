#pragma once
#include "../../SCBW/structures/CUnit.h"
#include "../../SCBW/structures.h"

namespace hooks {

	void stats_panel_display(BinDlg* dialog);										// 00426C60
	char* getUnitRankString(CUnit* a1);												// 0042F370
	void statsArmorLevel(BinDlg* dialog, int32 unused, uint16 buttonRelated);		// 00425600
	void statsWeaponLevel(BinDlg* dialog, uint16 buttonRelated, uint8 weaponId);	// 00425790

	void injectStatsPanelDisplayHook();

}