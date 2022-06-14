#pragma once
#include <SCBW/structures.h>

namespace hooks
{
	s32 decrementRemainingBuildTime(Unit* unit);

	void inject_build_time_hooks();
}