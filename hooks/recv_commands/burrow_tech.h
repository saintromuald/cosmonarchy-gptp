#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void cmdrecv_unburrow();				// 004C1AC0
	void cmdrecv_burrow(u8 command_type);	// 004C1FA0
	void unburrow(Unit* unit);				// 004E97C0

	void inject_burrow_tech_hooks();
}