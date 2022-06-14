#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Pickup1(Unit* unit);				// 004E7300
	void orders_Pickup3_0(Unit* unit);				// 004E73B0
	void orders_Pickup2(Unit* unit);				// 004E75D0
	void orders_MoveUnload(Unit* unit);			// 004E7700
	void orders_Pickup4_0(Unit* unit);				// 004E7B70
	void orders_EnterTransport(Unit* unit);		// 004E7CF0
	void orders_Unload(Unit* unit);				// 004E80D0
	bool can_enter_transport(Unit* a1, Unit* a2);	// 004E6E00

	void inject_load_unload_orders_hooks();

} //hooks