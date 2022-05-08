#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void remove_powerup_image_overlay(Unit* worker);								// 004F3900
	void transferResourceToWorkerHook(Unit* worker, Unit* resource);				// 004696D0
	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker);						// 00468930
	void order_enter_exit_gas(Unit* unit);											// 00469980
	void CanHarvestGas(Unit* unit);													// 00469000
	u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker);	// 004557C0
	void orders_harvest4(Unit* worker);												// 00468ED0
	s32 begin_harvest(Unit* worker, Unit* node);									// 00468CAA

	void injectHarvestResource();
}