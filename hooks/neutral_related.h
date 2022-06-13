#pragma once
#include <SCBW/api.h>
#include <SCBW/structures/Unit.h>

namespace hooks
{
	bool unit_is_not_neutral(CHKUnit* unit);		// 004CBE20
	void inject_neutral_related_hooks();
}