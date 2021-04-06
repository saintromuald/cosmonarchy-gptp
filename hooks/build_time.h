#pragma once
#include <SCBW/structures.h>

namespace hooks {
	int32 decrementRemainingBuildTime(CUnit* unit);

	void injectBuildTimeHooks();
}