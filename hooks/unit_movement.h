#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

	int32 unitMovementDormant(CUnit* unit);		// 0046A8D0
	void injectUnitMovementHooks();
}