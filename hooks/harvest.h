#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void transferResourceToWorkerHook(Unit* worker, Unit* resource);			// 004696D0
	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker);					// 00468930
	void order_enter_exit_gas(Unit* unit);										// 00469980
	void MoveToHarvestGas(Unit* unit);											// 00469000

	void injectHarvestResource();
}