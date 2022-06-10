#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	bool cmdrecv_place_building_allowed(Unit* builder, u8 order_id, u16 builtUnitId);			// 0048DBD0
	void cmdrecv_place_building_normal(s16 x, s16 y, u32 order_id, u32 builtUnitId);				// 0048E010
	void cmdrecv_place_addon(s16 x, s16 y, u32 order_id, u32 builtUnitId);				// 0048E0A0
	void cmdrecv_build(u8 order_id, s16 x, s16 y, u16 builtUnitId);								// 004C23C0
	void process_building_placement(Unit* unit, u8 order_id, Point16 pos, u16 target_unit_id);	// 0048DE70

	void inject_cmdrecv_build_hooks();
}