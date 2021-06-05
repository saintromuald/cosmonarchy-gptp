#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s8 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId);

	void injectNewRequirementsHooks();
}