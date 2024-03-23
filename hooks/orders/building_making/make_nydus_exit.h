#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks
{
	void orders_build5(Unit* nydus_canal); // 0045DC20
	u32 adjust_flags(u32 flags, u32 player);

	void inject_make_nydus_exit_hooks();
}