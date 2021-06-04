#pragma once
#include <SCBW/api.h>

namespace hooks
{

	s32 parseRequirementOpcodes(Unit* unit, u32 datReqOffset, u16 id, u32 player, u16* datReqBase); // 0046D610
	s8 UnitCreateAllowed(u16 unitId, Unit* unit, s32 playerID);										// 0046E1C0	
	void parseUnitsDatReqs();

	void injectRequirementHooks();
}