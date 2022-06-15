#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Die(Unit* unit);		//00479480

	void inject_die_order_hooks();

} //hooks