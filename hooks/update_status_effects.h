#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void update_status_effects(Unit* unit);
	void ai_burrower(Unit* unit);

	void injectUpdateStatusEffects();
}