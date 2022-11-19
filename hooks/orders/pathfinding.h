#pragma once
#include <SCBW/api.h>

namespace hooks
{
	b32 __fastcall is_point_at_unit_border(u32 unused, Unit* target_unit, Unit* unit, Point16 target_point);	// 0042DA90
	s32 move_to_target(Unit* target, Unit* unit);																// 004EB720
	b32 get_active_tile_flag(s16 y, s16 x, s32 includedFlags, s32 excludedFlags);								// 00472F80
	bool unit_has_path_to_dest(Unit* unit, s32 x, s32 y);														// 0049CB60
	bool unit_has_path_to_unit(Unit* pather, Unit* target);														// 0049CBB0
	void set_repulse_angle(Unit* unit);																			// 00453300
	void repulse_unit(Unit* unit, b32 should_repulse);															// 004535A0

	void inject_pathfinding_hooks();
}