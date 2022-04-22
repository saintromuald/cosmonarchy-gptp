#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 createUnitAllowed_Partial(s16 unitId, Unit* unit, s32 playerId);
	s32 createUnitIdAllowed(s16 unitId, s32 player_id);

	void injectNewRequirementsHooks();
}