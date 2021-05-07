#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_MedicHoldPosition(Unit* unit);		//00464050  orders_MedicHoldPosition
	void orders_ReaverStop(Unit* unit);			//004654B0  orders_ReaverStop
	void orders_Guard(Unit* unit);					//00475B90  orders_Guard
	void function_004774A0(Unit* unit);			//004774A0	sub_4774A0_unreferenced
	void orders_TurretGuard(Unit* unit);			//004777F0  orders_TurretGuard

	void injectStopHoldPosOrdersHooks();

} //namespace hooks
