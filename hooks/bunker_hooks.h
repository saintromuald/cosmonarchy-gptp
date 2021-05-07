#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	void unitAttackFromInsideBunkerHook(Unit* unit);
	void createBunkerAttackThingyHook(Unit* unit);

	void injectBunkerHooks();

} //hooks
