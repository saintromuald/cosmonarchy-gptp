#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	/*
	getSuppliesRemaining		004889D0
	getSuppliesMax				00488990
	getSuppliesUsed				004888C0
	InitializeUnitCounts		00488F90
	StatRes_MainProc			004E5640
	sub_4246D0					004246D0
	sub_425230					00425230
	sub_427540					00427540
	*/

	u32 getSuppliesAvailable(u8 playerId, s32 race);				// 00488900

	void injectSupplyHooks();
}