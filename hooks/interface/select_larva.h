#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	u32 getLarvaeUnitsFromListHook(Unit** unitList, Unit** outputList);	// 00423190
	void BTNSACT_SelectLarva();													// 00423930

	void injectSelectLarvaHooks();

}