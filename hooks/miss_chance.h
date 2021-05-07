#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	s32 canHitTargetCheck(Unit* a1, Unit* a2);	// 004765B0
	void injectMissChanceHooks();
}