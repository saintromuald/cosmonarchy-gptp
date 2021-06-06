#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId);

	void injectNewRequirementsHooks();
}