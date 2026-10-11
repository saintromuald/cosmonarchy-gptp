#pragma once
#include "../BW/structures/Unit.h"

namespace hooks
{
	b32 unit_can_recharge_shields(Unit* target, Unit* battery);	// 00493520
	void order_recharge_shields(Unit* unit);					// 00493DD0

	void inject_recharge_shields_hooks();
}