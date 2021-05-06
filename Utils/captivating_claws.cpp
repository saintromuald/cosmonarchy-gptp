#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

u32 captivatingClawsGetStacks(CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	return std::min(targetExt->unitVectors[UnitVectorCaptivatingClaws].size(), (u32)4);
}

u32 captivatingClawsGetTimer(CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	return targetExt->timers[TimerCaptivatingClaws];
}

void captivatingClawsApply(CUnit* source, CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	targetExt->timers[TimerCaptivatingClaws] = 1 * 24 + 1;
	auto& vec = targetExt->unitVectors[UnitVectorCaptivatingClaws];
	auto find = std::find(vec.begin(), vec.end(), source);
	if (find == vec.end())
	{
		vec.push_back(source);
	}
}