#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

b32 unitCanRechargeShieldsHook(Unit* target, Unit* battery);
void orderRechargeShieldsHook(Unit* unit);

void injectRechargeShieldsHooks();

} //hooks
