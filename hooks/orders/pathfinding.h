#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 moveToTarget(Unit* target, Unit* unit); // 004EB720

	void injectPathfindingHooks();
}