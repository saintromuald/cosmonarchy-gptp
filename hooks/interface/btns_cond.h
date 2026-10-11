#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks {
s32 BTNSCOND_NoNydusExit(Unit* unit); // 00428340
s32 BTNSCOND_Movement(Unit* unit);	  // 004283C0
s32 BTNSCOND_HasScarabs(Unit* unit);  // 004286A0

void inject_btns_cond_hooks(); // Disabled in initialize.
} //hooks
