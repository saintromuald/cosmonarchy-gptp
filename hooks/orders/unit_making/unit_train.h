#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void secondaryOrd_TrainFighter(Unit* unit);		// 00466790
	void orderTrain(Unit* unit);					// 00468420

	b32 _stdcall ButtonIsTraining(Unit* unit);		// 00428530

	void injectUnitTrainHooks();
	s32 isQueueSlotActive(Unit* unit, u32 slot);
	char getMaxHangerSpace(Unit* a1);				// 004653D0
}