#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_medic_heal2(Unit* unit);						// 00463740
	void orders_heal_move(Unit* unit);							// 004637B0
	void orders_medic(Unit* unit);								// 00463900
	void orders_medic_heal1(Unit* unit);						// 00464180
	bool ai_acquire_heal_target(Unit* cleric, Unit* target);	// 00463530
	u32 do_medic_heal(Unit* medic, Unit* target);				// 00463C40

//	void orders_medic_hold_position(Unit* unit);		// Now located in base_orders/stopholdpos_orders.h
	void inject_medic_orders_hooks();
}