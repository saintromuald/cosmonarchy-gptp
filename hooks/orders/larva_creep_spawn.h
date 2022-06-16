#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void create_building_larva(Unit* main_building);													// 0049D660
	bool iterate_units_at_location_target_proc_larva_count(Unit* unit, Unit* main_building);			// 004E8C80
	void secondary_order_spawning_larva(Unit* unit);													// 004EA780
	void secondary_order_spread_creep_spawning_larva(Unit* unit);										// 004EA880

	bool get_creep_region(Box32* rect, u16 unit_id, s32 x, s32 y, bool isComplete);						// 00413DB0
	bool spreads_creep(u16 unit_id, bool baseRetVal);													// 00413870
	bool in_creep_area(s32 x, s32 y);																	// inline function
	bool run_creep_randomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);	// 004140A0
//	char apply_creep_at_location_from_unit_id(s16 a1, s32 a2, s32 a3);									// 004148F0
	u32 btsncond_zerg_building_rally(u32 unused, u32 player, Unit* unit);								// 00429740
	s32 update_creep_disappearance_unit(Unit* unit);													// 0047DD60

	void inject_larva_creep_spawn_hooks();
}
