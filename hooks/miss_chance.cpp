#include "miss_chance.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s32 can_hit_target_check(Unit* target, Unit* attacker)
	{
		if (bw::is_under_carapace_swarm(target)) return 255;
		return 0;
	};
}