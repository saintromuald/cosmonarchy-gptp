#pragma once
#include "../../SCBW/structures/CUnit.h"
#include "../../SCBW/structures.h"

namespace hooks
{

	void stats_panel_display(BinDlg* dialog);										// 00426C60
	char* getUnitRankString(CUnit* a1);												// 0042F370
	void drawBuildingInfo(BinDlg* dlg);												// 00427890
	void statsArmorLevel(BinDlg* dialog, s32 unused, u16 buttonRelated);			// 00425600
	void statsWeaponLevel(BinDlg* dialog, u16 buttonRelated, u8 weaponId);			// 00425790

	void injectStatsPanelDisplayHook();

}