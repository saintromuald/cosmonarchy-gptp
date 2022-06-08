#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 create_unit_id_allowed(s16 unit_id, s32 player_id, bool ignore_creator);
	s32 create_unit_allowed_partial(s16 unit_id, Unit* unit, s32 playerId);
	bool can_set_rally_opcode(Unit* unit);

	void injectNewRequirementsHooks();
}