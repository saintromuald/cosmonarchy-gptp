#pragma once
#include "SCBW\api.h"

namespace hooks {
	u32 get_order_from_unit_id(u32 unit_id);
	void injectEconomicUnitHooks();
}