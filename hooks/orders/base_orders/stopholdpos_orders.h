#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_medic_hold_position(Unit* unit);		// 00464050
	void orders_reaver_stop(Unit* unit);				// 004654B0
	void orders_guard(Unit* unit);						// 00475B90
	void orders_player_guard(Unit* unit);				// 004774A0
	void orders_turret_guard_internal(Unit* unit);		// 004777F0

	void inject_stop_hold_position_orders_hooks();
}