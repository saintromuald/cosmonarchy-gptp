#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	void unitAttackFromInsideBunkerHook(Unit* unit);
	void createBunkerAttackThingyHook(Unit* unit);

	void inject_bunker_hooks();

} //hooks
