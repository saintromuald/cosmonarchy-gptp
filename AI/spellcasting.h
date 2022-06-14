#pragma once
#include <AI/ai_common.h>

namespace AI {

bool AI_spellcasterHook(Unit* unit, bool isUnitBeingAttacked);

} //AI

namespace hooks {

void inject_spellcaster_ai_hooks();

} //hooks