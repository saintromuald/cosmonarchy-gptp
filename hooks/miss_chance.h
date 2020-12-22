#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int canHitTargetCheck(CUnit* a1, CUnit* a2);	// 004765B0
	void injectMissChanceHooks();
}