#pragma once
#include <SCBW/api.h>
#include <SCBW/structures/CUnit.h>

namespace hooks {

	bool unitNotNeutral(CHKUnit* unit);								// 004CBE20
	bool shouldAttackUnit(CUnit* attacker, CUnit* target);			// 00431550
	void injectNeutralRelatedHooks();
}