#include "miss_chance.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>

namespace hooks
{
	s32 can_hit_target_check(Unit* target, Unit* attacker)
	{
		if (bw::is_under_carapace_swarm(target)) return 255;
		return 0;
	};
}