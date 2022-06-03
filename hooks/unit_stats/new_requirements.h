#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 createUnitAllowed_Partial(s16 unit_id, Unit* unit, s32 playerId);
	s32 createUnitIdAllowed(s16 unit_id, s32 player_id, bool ignore_creator);

	void injectNewRequirementsHooks();
}