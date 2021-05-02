#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks
{

    void secondaryOrd_TrainFighter(CUnit* unit);	// 0x00466790
    void function_00468420(CUnit* unit);			// 0x00468420

    void injectUnitTrainHooks();
    s32 isQueueSlotActive(CUnit* unit, u32 slot);
    char getMaxHangerSpace(CUnit* a1);				// 0x004653D0
} //hooks