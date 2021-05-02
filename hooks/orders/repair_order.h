#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Repair1(CUnit* unit);																// 004673D0
	s32 isOrderValid(char orderId, CUnit* unit, s32 unused, CUnit* target, s32 bErrorFlag);			// 00474D90
	s32 orderAllowed(CUnit* unit, u16 orderId, s32 player);											// 0046DC20

	void injectRepairOrderHook();

} //hooks