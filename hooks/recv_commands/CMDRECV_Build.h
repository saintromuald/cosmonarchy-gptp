#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks
{
	bool cmdrecv_place_building_allowed(Unit* builder, u8 order_id, u16 built_unit_id);			// 0048DBD0
	void cmdrecv_place_building_normal(s16 x, s16 y, u32 order_id, u32 built_unit_id);			// 0048E010
	void cmdrecv_place_addon(s16 x, s16 y, u32 order_id, u32 built_unit_id);					// 0048E0A0
	void cmdrecv_build(u8 order_id, s16 x, s16 y, u16 built_unit_id);							// 004C23C0
	void process_building_placement(Unit* unit, u8 order_id, v2u16 pos, u16 target_unit_id);	// 0048DE70
	u32 cmdrecv_place_building(u32 order_id, u16 built_unit_id, v2u16 position);				// 0048E190
	b32 check_resources_for_building(u32 notify, u8 player);									// 0042D0C0
	void display_error_message_id_with_sfx(u32 player, u32 message_id, u32 sound_id);			// 0048EE30

	void inject_cmdrecv_build_hooks();
}