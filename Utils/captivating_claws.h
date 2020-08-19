#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace captivating_claws {
	u32 getCaptivatingClawsStacks(CUnit* target);
	void applyCaptivatingClaws(CUnit* source, CUnit* target);
	void clearCaptivatingClaws(CUnit* target);
}
