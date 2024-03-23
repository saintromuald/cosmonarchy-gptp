#pragma once
#include "BW/structures.h"
#include "graphics/Font.h"

#define ResSize 32//PLAYABLE_PLAYER_COUNT * 4
#define RowSize 16

extern char stat_res_resources_buffer[ResSize][16];
extern char stat_res_production_buffer[PlayerLimit][RowSize][4];
extern s32 stat_res_displayed_gas[PlayerLimit];						// Reformat of "unknown_68C22C"
extern s32 stat_res_displayed_minerals[PlayerLimit];				// Reformat of "unknown_68C230"
struct ProductionArrays {
	u16 types[PlayerLimit][RowSize];
	u16 timers[PlayerLimit][RowSize];
	u8 counts[PlayerLimit][RowSize];
};
extern ProductionArrays production;
extern graphics::Font* font_stat_res;

namespace hooks
{
	u32 get_supplies_available(u8 player, s32 race);								// 00488900
	s32 get_supplies_remaining(u8 player, u16 unit_id);								// 004889D0
	s32 get_supplies_max(u16 unit_id, u8 player);									// 00488990
	s32 get_supplies_used(s32 race, u8 player);										// 004888C0
	char initialize_unit_counts();													// 00488F90
	s32 get_supplies_temp();														// 004246D0
	bool redraw_supply_info();														// 00425230
	void draw_supply_unit_status(BinDlg* dialog);									// 00427540

	bool has_supplies_for_unit(u8 player, u16 unit_id, bool can_show_error);		// 0042CF70
	void increment_unit_scores(Unit* unit, s32 count);								// 00488BF0
	void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores);		// 00488D50

	void __fastcall stat_res_main_proc(BinDlg* dialog);								// 004E5640
	//bool __fastcall stat_res_text_interact(BinDlg* dialog, DialogEvent* dlg_event);			// 00418E60
	void __fastcall stat_res_text_update(BinDlg* dialog, int dummy0, int dummy1, int dummy2);	// 004E5460
	void __fastcall stat_res_colortext_update(BinDlg* dialog, int dummy0, int dummy1, int dummy2);// Version of stat_res_text_update without graphic display.
	bool __fastcall stat_res_custom_ctrl(BinDlg* dialog, DialogEvent* dlg_event);				// 004E5850
	void stat_res_register_user_dialog_action(/*BinDlg* dialog*/);// Version of register_user_dialog_action tailored to statres.
	void stat_res_refresh_production_arrays();
	void __fastcall stat_res_get_production(Unit* unit);

	void inject_supply_hooks();
}