#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 init_placement_box_pos(s32 unused, Point16* placement);						// 0048D660
	u8 draw_placement_box(Point16 ecx0);											// 0048DDC0
	s32 update_placement_state_for_placement_box();									// 0048DCE0
	s32 is_tile_blocked_by(Unit** unit_array, Unit* builder, s32 x_tile, s32 y_tile, s8 dont_ignore_reacting, s8 also_invisible); // 00473300
	int update_creep_building_placement_state(u8 player, u16 x_tile, u16 y_tile, Point16 size_wh, u32 placement_state_entry, u32 check_vision, u32 check_vision_2); // 00473010

	void injectPlacementHooks();
}