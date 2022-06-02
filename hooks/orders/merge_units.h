#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length);	//004BFA80

	void orders_CompletingArchonSummon(Unit* unit);									//00493B10
	void orders_WarpingDarkArchon(Unit* unit);											//00494690
	void orders_WarpingArchon(Unit* unit);												//004948B0

	//The injector function that should be called in initialize.cpp
	void injectMergeUnitsHooks();

} //hooks
