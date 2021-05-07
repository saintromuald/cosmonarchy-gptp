#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_NukeTrack(CUnit* unit);	//00464D10
	void orders_NukeGround(CUnit* unit);//00479200

	void injectNukeOrdersHooks();

} //hooks