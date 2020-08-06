#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int isUnderDarkSwarm(CUnit* a1);				// 00476210
	int canHitTargetCheck(CUnit* a1, CUnit* a2);	// 004765B0
	void injectMissChanceHooks();
}