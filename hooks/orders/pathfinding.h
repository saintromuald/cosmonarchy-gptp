#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 move_to_target(Unit* target, Unit* unit);									// 004EB720
	b32 get_active_tile_flag(s16 y, s16 x, s32 includedFlags, s32 excludedFlags);	// 00472F80
	bool unitHasPathToDest(Unit* unit, s32 x, s32 y);							// 0049CB60
	bool unit_has_path_to_unit(Unit* pather, Unit* target);							// 0049CBB0

	void inject_pathfinding_hooks();
}