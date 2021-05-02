#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Repair1(CUnit* unit);																// 004673D0
	s32 isOrderValid(char orderId, CUnit* unit, s32 unused, CUnit* target, s32 bErrorFlag);	// 00474D90

	void injectRepairOrderHook();

} //hooks