#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

u32 captivatingClawsGetStacks(CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	return std::min(targetExt->CaptivatingClawsUnitVector.size(), (u32)4);
}

u32 captivatingClawsGetTimer(CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	return targetExt->CaptivatingClawsTimer;
}

void captivatingClawsApply(CUnit* source, CUnit* target)
{
	UnitExt* targetExt = globals.GetUnitExt(target);
	targetExt->CaptivatingClawsTimer = 1 * 24 + 1;
	auto& vec = targetExt->CaptivatingClawsUnitVector;
	auto find = std::find(vec.begin(), vec.end(), source);
	if (find == vec.end())
	{
		vec.push_back(source);
	}
}