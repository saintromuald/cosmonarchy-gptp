#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks
{
	void orders_die(Unit* unit);		// 00479480

	void inject_die_order_hooks();
}