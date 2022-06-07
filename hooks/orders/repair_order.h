#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_repair_1(Unit* unit);																// 004673D0
	s32 is_order_valid(char orderId, Unit* unit, s32 unused, Unit* target, s32 bErrorFlag);			// 00474D90
	s32 order_allowed(Unit* unit, u16 orderId, s32 player);											// 0046DC20
	s32 show_right_click_error_if_needed(Unit* target);												// 00455A00

	void inject_repair_order_hooks();
}