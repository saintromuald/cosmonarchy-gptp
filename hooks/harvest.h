#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void transferResourceToWorkerHook(Unit* worker, Unit* resource);			// 004696D0
	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker);					// 00468930
	void order_enter_exit_gas(Unit* unit);										// 00469980
	void CanHarvestGas(Unit* unit);												// 00469000
	u32 AI_getVespeneType(Unit* unit, u32 index);								// 004326E0

	void injectHarvestResource();
}