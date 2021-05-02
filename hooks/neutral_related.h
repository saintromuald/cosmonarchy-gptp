#pragma once
#include <SCBW/api.h>
#include <SCBW/structures/CUnit.h>

namespace hooks
{

	bool unitNotNeutral(CHKUnit* unit);								// 004CBE20
	void injectNeutralRelatedHooks();
}