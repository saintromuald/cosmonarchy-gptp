#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Build5(Unit* nydus_canal); //0x0045DC20
	u32 adjustFlags(u32 flags, u32 player);
	void injectMakeNydusExitHook();

} //hooks