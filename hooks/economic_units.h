#pragma once
#include "SCBW\api.h"

namespace hooks {
	uint32 get_order_from_unit_id(uint32 unit_id);
	void injectEconomicUnitHooks();
	void Ai_BuildSupplies(AiTown* town);
	uint32 get_worker_from_race(uint32 race);
	uint32 get_townhall_from_race(uint32 race);
	bool unit_is_correct_townhall(uint32 unit_id, uint32 cmp_id, CUnit* unit);
	bool is_townhall(uint32 unit_id);
	bool is_terran_supply(uint16 unit_id, CUnit* worker);
	bool is_vespene_top_structure(uint16 unit_id);
	bool is_gasholder_structure(uint16 unit_id);
	bool checkSupplyNeed(uint8 player_, uint8 race_);
	int32 AI_AddTownSpendingRequest(int32 playerId, int32 priority, int32 unitId, void* param); // 00448400

	uint32 orders_isGeyserTopUnitHook(CUnit* unit);
}