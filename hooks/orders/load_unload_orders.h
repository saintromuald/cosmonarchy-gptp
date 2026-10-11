#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks {
void orders_pickup_1(Unit* unit);						// 004E7300
void orders_pickup_3(Unit* unit);						// 004E73B0
void orders_pickup_2(Unit* unit);						// 004E75D0
void orders_move_unload(Unit* unit);					// 004E7700
void orders_pickup_4(Unit* unit);						// 004E7B70
void orders_enter_transport(Unit* unit);				// 004E7CF0
void orders_unload(Unit* unit);							// 004E80D0
bool can_enter_transport(Unit* transport, Unit* unit);	// 004E6E00

void inject_load_unload_orders_hooks();
}