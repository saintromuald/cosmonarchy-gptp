#pragma once
#include <BW/api.h>
#include <BW/extender/TileHeight_extender.h>
#include <BW/bwdata.h>
#include <BW/structures.h>
#include <BW/structures/Unit.h>

inline u8 vision_timers[256*256][8];

namespace hooks {
	inline void refresh_vision_timers(u32 test_tile_pos, u8 vision_flags, u8 timer);
	void __fastcall ground_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void __fastcall ground_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void __fastcall air_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void __fastcall air_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void __fastcall hit_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void __fastcall hit_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags);
	void update_fog();
	void reveal_sight_at_location(u32 sight_range, u32 vision_flags, s32 pos_x, s32 pos_y, bool is_air_unit);
	s32 get_ground_height_at_px(s32 offset_y, s32 offset_x);

	void inject_sight_update_hooks();
}