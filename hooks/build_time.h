#pragma once
#include <BW/structures.h>

namespace hooks
{
	s32 decrementRemainingBuildTime(Unit* unit);

	void inject_build_time_hooks();
}