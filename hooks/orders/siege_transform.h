#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_tank_mode(Unit* unit);
	void orders_siege_mode(Unit* unit);

	void inject_siege_transform_hooks();
}