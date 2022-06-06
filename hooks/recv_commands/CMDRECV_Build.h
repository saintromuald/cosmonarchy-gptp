#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	bool cmdrecv_place_building_allowed(Unit* builder, u8 orderId, u16 builtUnitId);	//0048DBD0
	void cmdrecv_place_building_normal(s16 x, s16 y, u32 orderId, u32 builtUnitId);	//0048E010
	void cmdrecv_place_building_addon(s16 x, s16 y, u32 orderId, u32 builtUnitId);	//0048E0A0
	void CMDRECV_Build(u8 orderId, s16 x, s16 y, u16 builtUnitId);					//004C23C0

	void injectCMDRECV_BuildHooks();

} //hooks
