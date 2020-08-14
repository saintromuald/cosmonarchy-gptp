#pragma once
#include <SCBW/structures.h>

namespace hooks {
	s32 decrementRemainingBuildTime(CUnit* unit);

	void injectBuildTimeHooks();
}