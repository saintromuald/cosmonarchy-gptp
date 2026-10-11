#pragma once
#include "BW\api.h"

namespace hooks {
	u32 get_build_order_from_unit_id(u32 unit_id);
	void ai_build_supplies(AiTown* town);
	bool unit_is_correct_townhall(u32 unit_id, u32 cmp_id, Unit* unit);
	bool is_town_center(u32 unit_id);
	bool is_terran_supply(u16 unit_id, Unit* worker);
	bool check_supply_need(u8 player_, u8 race_);
	s32 ai_add_town_spending_request(s32 player, s32 priority, s32 unit_id, void* param); // 00448400
	u32 orders_is_geyser_cap(Unit* unit);

	void inject_economic_unit_hooks();
}