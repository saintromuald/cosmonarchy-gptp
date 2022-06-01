#pragma once
#include "../../SCBW/structures/Unit.h"
#include "../../SCBW/structures.h"

namespace hooks
{
	void stats_panel_display(BinDlg* dialog);										// 00426C60
	char* get_unit_rank_string(Unit* a1);												// 0042F370
	void draw_unit_kills_string(BinDlg* dialog);                                       // 00425DD0
	void draw_building_info(BinDlg* dlg);												// 00427890
	void stats_armor_level(BinDlg* dialog, s32 unused, u16 buttonRelated);			// 00425600
	void statsWeaponLevel(BinDlg* dialog, u16 buttonRelated, u8 weaponId);			// 00425790
	const char* getUnitName(Unit* unit);											// 0047B5A0
	void set_tooltip_info_unit(u32 unit_id);										// 00459531

	void injectStatsPanelDisplayHook();
}