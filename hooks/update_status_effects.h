#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void update_status_effects(Unit* unit);			// 00492F70
	void ai_burrower(Unit* unit);					// 004A1340
	bool is_revealed(Unit* unit);					// 00491810

	void inject_update_status_effects_hooks();
}