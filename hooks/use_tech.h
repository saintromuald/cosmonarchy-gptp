#pragma once
#include <SCBW/structures.h>

namespace hooks
{
	void btnsact_use_tech(u16 tech, GrpHead* head);				// 
	u32 tech_use_allowed(Unit* unit, u16 tech_id, u32 player);	// 0046DD80
	void inject_use_tech_hooks();
}