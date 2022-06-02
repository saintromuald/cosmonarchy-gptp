#include "miss_chance.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s32 canHitTargetCheck(Unit* a1, Unit* a2)
	{
		if (scbw::is_under_dark_swarm(a1)) return 255;
		return 0;
	};
}