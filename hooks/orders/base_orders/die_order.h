#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Die(CUnit* unit);		//00479480

	void injectDieOrderHook();

} //hooks