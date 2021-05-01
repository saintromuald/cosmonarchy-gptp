#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Repair1(CUnit* unit);																// 004673D0
	int32 isOrderValid(char orderId, CUnit* unit, int32 unused, CUnit* target, int32 bErrorFlag);	// 00474D90

	void injectRepairOrderHook();

} //hooks