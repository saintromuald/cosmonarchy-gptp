#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks {
void orders_reaver_attack(Unit* unit);											// 00465690
void orders_carrier_attack(Unit* unit);											// 00465950
void orders_attack_fixed_range(Unit* unit);										// 00477D00
void orders_sap_unit(Unit* unit);												// 004788E0
void orders_sap_location(Unit* unit);											// 00478A40
void orders_attack_move_ep(Unit* unit);											// 00478DE0
void orders_attack_move(Unit* unit);											// 00479040
void orders_attack_unit(Unit* unit);											// 00479BD0
void orders_strafe_unit2(Unit* unit);											// 00465E00
void set_interceptors_to_standby(Unit* unit);									// 00465780
void orders_strafe_unit(Unit* unit);											// 00465F60
void get_next_strafe_position(Unit* unit, u8 strafe_type, v2u16 position);	    // 00465D30
void orders_carrier_idle(Unit* unit);											// 004666A0
Unit* release_fighter(Unit* unit);												// 00466440 | local only!
s32 order_attack_behavior(Unit* unit);											// 00478540
b32 attack_move_react_to_attack(Unit* unit, u32 order_id);						// 00478370
void order_attack_move_try_pick_target(Unit* unit, u8 order_id);				// 00477820
void orders_turret_attack(Unit* unit);											// 00477980
void orders_patrol(Unit* unit);													// 004780F0
void order_to_attack_target(Unit* target, Unit* unit, b32 process_subunit, b32 target_is_cloaked);	// 00476FC0
Unit* dispatch_fighter(Unit* carrier);											  // 00466440
b32 orders_carrier_reaver_idle(Unit* unit, Unit* target, u32 range, s32 range_2); // 00465A60

void inject_attack_orders_hooks();
}