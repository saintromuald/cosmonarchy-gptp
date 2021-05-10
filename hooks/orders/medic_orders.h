#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_MedicHeal2(Unit* unit);					// 00463740
	void orders_HealMove(Unit* unit);					// 004637B0
	void orders_Medic(Unit* unit);						// 00463900
//	void orders_MedicHoldPosition(Unit* unit);			// base_orders/stopholdpos_orders.h
	void orders_MedicHeal1(Unit* unit);					// 00464180
	bool AI_AcquireHealTarget(Unit* a1, Unit* a2);		// 00463530
	u32 doMedicHeal_Func(Unit* medic, Unit* target);	// 00463C40

	void injectMedicOrdersHooks();
}