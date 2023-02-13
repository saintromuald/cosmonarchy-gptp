#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_medic_hold_position(Unit* unit);		//00464050  orders_MedicHoldPosition
	void orders_reaver_stop(Unit* unit);			//004654B0  orders_ReaverStop
	void orders_guard(Unit* unit);					//00475B90  orders_Guard
	void orders_player_guard(Unit* unit);			//004774A0	sub_4774A0_unreferenced
	void orders_turret_guard(Unit* unit);			//004777F0  orders_TurretGuard

	void inject_stop_hold_position_orders_hooks();

} //namespace hooks
