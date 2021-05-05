#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

u32 captivatingClawsGetStacks(CUnit* target)
{
	UnitExt* unitExt = &globals.unitExt[target];
	return std::min(unitExt->CaptivatingClawsUnitVector.size(), (u32)4);
}

u32 captivatingClawsGetTimer(CUnit* target)
{
	UnitExt* unitExt = &globals.unitExt[target];
	return unitExt->CaptivatingClawsTimer;
}

void captivatingClawsApply(CUnit* source, CUnit* target)
{
	UnitExt* unitExt = &globals.unitExt[target];
	unitExt->CaptivatingClawsTimer = 1 * 24 + 1;
	auto& vec = unitExt->CaptivatingClawsUnitVector;
	auto find = std::find(vec.begin(), vec.end(), source);
	if (find == vec.end())
	{
		vec.push_back(source);
	}
}