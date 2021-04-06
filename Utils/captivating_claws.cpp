#include "captivating_claws.h"
#include "globals.h"
#include <algorithm>

uint32 captivatingClawsGetStacks(CUnit* target) {
	return std::min((int)scbw::get_generic_unit_timer_map(target, ValueId::CaptivatingClawsMap).size(), 4);
}

uint32 captivatingClawsGetHighestTimer(CUnit* target) {
	auto& map = scbw::get_generic_unit_timer_map(target, ValueId::CaptivatingClawsMap);
		
	int max = 0;
	for (auto& unit : map) {
		if (unit.second > max) {
			max = unit.second;
		} 
	}
	return max;
}

void captivatingClawsApply(CUnit* source, CUnit* target) {
	auto& map = scbw::get_generic_unit_timer_map(target, ValueId::CaptivatingClawsMap);

	if (map[source] == 0) {
		map[source] = (24 * 1) + 1;
	}
}