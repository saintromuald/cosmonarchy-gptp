#pragma once
#include "BW/structures/Unit.h"

namespace hooks {
void orders_id_cases(Unit* unit);						// 004EBED0
void secondary_orders_root(Unit* unit);					// 004EC170
void main_orders_root(Unit* unit);						// 004EC4D0
void do_next_queued_order(Unit* unit);					// 00475000
void orders_stay_in_range(Unit* unit);					// 0047C4F0
void partial_order_handling(Unit* unit);				// 004EBED0
s32 order_to_move_to_target(Unit* target, Unit* unit);	// 004EB980
void orders_nuke_launch(Unit* unit);					// 00464730
void targeted_order(u16 x_pos, u32 fow_unit_id, Unit* unit, u16 y_pos, u32 order_type, u32 is_queued,
                    u8 order_id, Unit* target, v2u16 position, Unit* secondary_target); // 004754F0
void force_order_done(Unit* unit);						// 00475420

void orders_grief_of_all_gods(Unit* unit);

void inject_orders_root_hooks();
}