#pragma once
#include "SCBW/structures.h"

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
	void stat_res_main_proc(BinDlg* dialog);										// 004E5640

	bool has_supplies_for_unit(u8 player, u16 unit_id, bool can_show_error);		// 0042CF70
	void increment_unit_scores(Unit* unit, s32 count);								// 00488BF0
	void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores);		// 00488D50

	void inject_supply_hooks();
}

// New arrays to support the enlarged statres.bin, for the replay changes
u32 StatRes_fxnInteract[] = {
	0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850,
	0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850,
	0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850,
	0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850, 0x004E5850 };	//:woozy_face:
u32 StatRes_accrue_gas[32];			// extension of "unknown_68C22C"
u32 StatRes_accrue_minerals[32];	// extension of "unknown_68C230"
char StatResTextBuffer[1024];		// [32 * 32]