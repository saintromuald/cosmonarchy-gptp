#pragma once
#include "SCBW/structures/Unit.h"

namespace hooks {
	void orders_id_cases(Unit* unit);		// 004EBED0
	void secondary_orders_root(Unit* unit);	// 004EC170
	void main_orders_root(Unit* unit);		// 004EC4D0

	void inject_orders_root_hooks();
}