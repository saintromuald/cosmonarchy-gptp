#pragma once
#include "SCBW\api.h"

namespace hooks {
	u32 get_order_from_unit_id(u32 unit_id);
	void injectEconomicUnitHooks();
	void Ai_BuildSupplies(AiTown* town);
	u32 get_worker_from_race(u32 race);
	u32 get_townhall_from_race(u32 race);
	bool is_townhall(u32 unit_id);
	bool is_terran_supply(u16 unit_id);
}