#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

u32 captivatingClawsGetStacks(Unit* target)
{
	UnitExt* targetExt = globals.get_unit_ext(target);
	return std::min(targetExt->unit_vectors[UnitVectorCaptivatingClaws].size(), (u32)4);
}

u32 captivatingClawsGetTimer(Unit* target)
{
	UnitExt* targetExt = globals.get_unit_ext(target);
	return targetExt->timers[TimerCaptivatingClaws];
}

void captivatingClawsApply(Unit* source, Unit* target)
{
	UnitExt* targetExt = globals.get_unit_ext(target);
	targetExt->timers[TimerCaptivatingClaws] = 1 * 24 + 1;
	auto& vec = targetExt->unit_vectors[UnitVectorCaptivatingClaws];
	auto find = std::find(vec.begin(), vec.end(), source);
	if (find == vec.end())
	{
		vec.push_back(source);
	}
}