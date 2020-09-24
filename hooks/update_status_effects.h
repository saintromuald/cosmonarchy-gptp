#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void updateStatusEffects(CUnit* unit);

void AI_Burrower(CUnit* a1);
void injectUpdateStatusEffects();

} //hooks
