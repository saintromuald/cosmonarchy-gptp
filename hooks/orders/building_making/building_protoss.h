#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_probe_build(Unit* unit);		// 004E4D00
	void orders_build_self_2(Unit* building);	// 004E4F40

	void inject_building_protoss_hooks();
}