#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{

	void unitAttackFromInsideBunkerHook(CUnit* unit);
	void createBunkerAttackThingyHook(CUnit* unit);

	void injectBunkerHooks();

} //hooks
