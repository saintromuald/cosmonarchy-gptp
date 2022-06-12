#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_nuke_track(Unit* unit);		// 00464D10
	void orders_nuke_ground(Unit* unit);	// 00479200

	void inject_nuke_orders_hooks();
}