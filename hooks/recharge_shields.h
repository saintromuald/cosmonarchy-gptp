#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

b32 unitCanRechargeShieldsHook(CUnit* target, CUnit* battery);
void orderRechargeShieldsHook(CUnit* unit);

void injectRechargeShieldsHooks();

} //hooks
