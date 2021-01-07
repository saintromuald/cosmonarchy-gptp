#pragma once
#include "SCBW\api.h"

namespace hooks {
	u32 get_order_from_unit_id(u32 unit_id);
	void injectEconomicUnitHooks();
	void Ai_BuildSupplies(AiTown* town);
	u32 get_worker_from_race(u32 race);
	u32 get_townhall_from_race(u32 race);
	bool unit_is_correct_townhall(u32 unit_id, u32 cmp_id, CUnit* unit);
	bool is_townhall(u32 unit_id);
	bool is_terran_supply(u16 unit_id, CUnit* worker);
	bool is_vespene_top_structure(u16 unit_id);
	bool is_gasholder_structure(u16 unit_id);
	bool checkSupplyNeed(u8 player_, u8 race_);
	s32 AI_AddTownSpendingRequest(s32 playerId, s32 priority, s32 unitId, void* param); // 00448400

	u32 orders_isGeyserTopUnitHook(CUnit* unit);
}