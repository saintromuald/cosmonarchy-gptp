#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Build5(CUnit* nydus_canal); //0x0045DC20
	uint32 adjustFlags(uint32 flags, uint32 player);
	void injectMakeNydusExitHook();

} //hooks