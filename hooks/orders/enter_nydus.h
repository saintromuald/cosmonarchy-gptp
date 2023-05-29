#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	bool can_enter_nydus(Unit* unit, Unit* nydus);		// 004E8C20
	void enter_nydus_effect(Unit* unit, Unit* nydus);	// 004EA180
	void orders_enter_nydus(Unit* unit);				// 004EA3E0

	void inject_enter_nydus_hooks();
}