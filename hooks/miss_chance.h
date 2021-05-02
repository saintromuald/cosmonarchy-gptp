#pragma once
#include "SCBW/structures.h"

namespace hooks
{

    s32 canHitTargetCheck(CUnit* a1, CUnit* a2);	// 004765B0
    void injectMissChanceHooks();
}