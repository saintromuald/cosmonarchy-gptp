#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 moveToTarget(Unit* target, Unit* unit);									// 004EB720
	b32 getActiveTileFlag(s16 y, s16 x, s32 includedFlags, s32 excludedFlags);	// 00472F80

	void injectPathfindingHooks();
}