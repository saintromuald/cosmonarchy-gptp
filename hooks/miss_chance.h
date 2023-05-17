#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	s32 can_hit_target_check(Unit* target, Unit* attacker);	// 004765B0

	void inject_miss_chance_hooks();
}