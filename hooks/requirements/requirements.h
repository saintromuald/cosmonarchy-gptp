#pragma once
#include <SCBW/api.h>

namespace hooks
{

	s32 parse_requirement_opcodes(Unit* unit, u32 datReqOffset, u16 id, u32 player, u16* datReqBase); // 0046D610
	s8 unit_create_allowed(u16 unit_id, Unit* unit, s32 playerID);										// 0046E1C0	
	void parse_unit_dat_requirements();

	void inject_requirement_hooks();
}