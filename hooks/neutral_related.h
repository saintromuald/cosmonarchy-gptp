#pragma once
#include <BW/api.h>
#include <BW/structures/Unit.h>

namespace hooks {
bool unit_is_not_neutral(ChkUnit* unit);		// 004CBE20
void inject_neutral_related_hooks();
} // namespace hooks