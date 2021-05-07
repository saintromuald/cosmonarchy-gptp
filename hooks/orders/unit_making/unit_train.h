#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void secondaryOrd_TrainFighter(CUnit* unit);	// 00466790
	void orderTrain(CUnit* unit);					// 00468420

	void injectUnitTrainHooks();
	s32 isQueueSlotActive(CUnit* unit, u32 slot);
	char getMaxHangerSpace(CUnit* a1);				// 004653D0
}