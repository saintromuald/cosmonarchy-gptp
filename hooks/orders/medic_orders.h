#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_MedicHeal2(CUnit* unit);				//00463740
	void orders_HealMove(CUnit* unit);					//004637B0
	void orders_Medic(CUnit* unit);						//00463900
//	void orders_MedicHoldPosition(CUnit* unit);			//base_orders/stopholdpos_orders.h
	void orders_MedicHeal1(CUnit* unit);				//00464180
//	void findHealTarget(CUnit* unit);					//004422A0
	bool AI_AcquireHealTarget(CUnit* a1, CUnit* a2);	//00463530
	u32 doMedicHeal_Func(CUnit* medic, CUnit* target);	//00463C40
	void injectMedicOrdersHooks();

} //hooks