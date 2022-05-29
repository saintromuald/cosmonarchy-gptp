#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void updateStatusEffects(Unit* unit);
	void AI_Burrower(Unit* unit);

	void injectUpdateStatusEffects();
}