#include "miss_chance.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "SCBW/api.h"

namespace hooks
{
	s32 canHitTargetCheck(CUnit* a1, CUnit* a2)
	{
		if (scbw::isUnderDarkSwarm(a1)) return 255;
		return 0;
	};
}