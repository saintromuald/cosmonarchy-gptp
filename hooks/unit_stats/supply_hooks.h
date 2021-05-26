#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	/*
	drawSupplyUnitStatus		00427540
	*/

	u32 getSuppliesAvailable(u8 playerId, s32 race);				// 00488900
	s32 getSuppliesRemaining(u8 playerId, u16 unitId);				// 004889D0
	s32 getSuppliesMax(u16 unitId, u8 playerId);					// 00488990
	s32 getSuppliesUsed(s32 race, u8 playerId);						// 004888C0
	char initializeUnitCounts();									// 00488F90
	void StatRes_MainProc(BinDlg* dialog);							// 004E5640
	s32 getSuppliesTemp();											// 004246D0
	bool redrawSupplyInfo();										// 00425230

	void injectSupplyHooks();
}