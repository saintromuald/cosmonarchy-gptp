#pragma once
#include <AI/ai_common.h>

namespace AI {
	bool ai_spellcaster_hook(Unit* unit, bool is_unit_being_attacked);
}

namespace hooks {
	void inject_spellcaster_ai_hooks();
}