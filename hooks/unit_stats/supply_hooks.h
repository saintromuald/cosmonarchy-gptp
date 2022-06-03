#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	u32 get_supplies_available(u8 playerId, s32 race);								// 00488900
	s32 get_supplies_remaining(u8 playerId, u16 unit_id);								// 004889D0
	s32 getSuppliesMax(u16 unit_id, u8 playerId);									// 00488990
	s32 get_supplies_used(s32 race, u8 playerId);										// 004888C0
	char initializeUnitCounts();													// 00488F90
	void StatRes_MainProc(BinDlg* dialog);											// 004E5640
	s32 getSuppliesTemp();															// 004246D0
	bool redrawSupplyInfo();														// 00425230
	void drawSupplyUnitStatus(BinDlg* dialog);										// 00427540

	bool hasSuppliesForUnitHook(u8 playerId, u16 unit_id, bool canShowErrorMessage); // 0042CF70
	void increment_unit_scores(Unit* unit, s32 count);								// 00488BF0
	void increment_unit_scores_ex(s32 count, Unit* unit, s32 increment_scores);		// 00488D50

	void injectSupplyHooks();
}