#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_Repair1(Unit* unit);																// 004673D0
	s32 isOrderValid(char orderId, Unit* unit, s32 unused, Unit* target, s32 bErrorFlag);			// 00474D90
	s32 orderAllowed(Unit* unit, u16 orderId, s32 player);											// 0046DC20
	s32 show_right_click_error_if_needed(Unit* target);												// 00455A00

	void injectRepairOrderHook();
}