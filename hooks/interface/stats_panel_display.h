#pragma once
#include "../../SCBW/structures/Unit.h"
#include "../../SCBW/structures.h"

namespace hooks
{
	void stats_panel_display(BinDlg* dialog);																									// 00426C60
	char* get_unit_rank_string(Unit* a1);																										// 0042F370
	void draw_unit_kills_string(BinDlg* dialog);																								// 00425DD0
	void draw_building_info(BinDlg* dlg);																										// 00427890
	void stats_armor_level(BinDlg* dialog, s32 unused, u16 buttonRelated);																		// 00425600
	void stats_weapon_level(BinDlg* dialog, u16 buttonRelated, u8 weaponId);																	// 00425790
	const char* get_unit_name(Unit* unit);																										// 0047B5A0
	void reset_tooltip_info();																													// 00459360 004593B0 004597D5
	void set_tooltip_info(u32 mineral_cost, u32 gas_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u32 z_supply);	// 00458800
	void set_tooltip_info_unit(u32 unit_id);                                                                                                    // 00459531
	void draw_tooltip_with_icon(Box16* area);																									// 00458EF0
	s32 stat_data_unit_wireframe_selection(BinDlg* dialog, Event* event_arg);																	// 004583E0
	void stat_data_unit_wireframe_select_update(BinDlg* dialog, int unused_1, int unused_2);													// 00456F50

	void injectStatsPanelDisplayHook();
}