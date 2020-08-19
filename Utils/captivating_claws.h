#pragma once
#include "SCBW/scbwdata.h"
#include "SCBW/api.h"

namespace captivating_claws {
	u32 getCaptivatingClawsStacks(CUnit* target);
	void addCaptivatingClawsSource(CUnit* source, CUnit* target);
	bool canApplyCaptivatingClaws(CUnit* source, CUnit* target);
	void clearCaptivatingClawsSources(CUnit* target);
}
