#include "update_unit_sight.h"
#include "BW/ExtendSightLimit.h"

static s32 g_temp_height;

namespace hooks {
	SightCallbacks g_sight_callbacks[]{ground_sight_cb_inside, air_sight_cb_inside, hit_sight_cb_inside, ground_sight_cb_edge, air_sight_cb_edge, hit_sight_cb_edge};

	inline void refresh_vision_timers(u32 test_tile_pos, u8 vision_flags, u8 timer) {
		s32 player = 7;
		u8 player_flag = 0x80;
		u8* cur_vision_timers = &vision_timers[test_tile_pos][0];
		do {
			if (vision_flags & player_flag && *(cur_vision_timers + player) < timer) {
				*(cur_vision_timers + player) = timer;
			}
			player_flag >>= 1;
		} while (--player >= 0);
	}

	void __fastcall ground_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_values->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_values->revealed_count;
		while (revealed_count) {
			sighted_tile->visibility_mask = 0xFF;
			if (tile_x + sighted_tile->w < g_map_tile_size->width && tile_y + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
				sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			}
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sight_values->masked_count;
		while (masked_count) {
			sighted_tile->visibility_mask = 0xFF;
			if (tile_x + sighted_tile->w < g_map_tile_size->width && tile_y + sighted_tile->h < g_map_tile_size->height) {
				u32 count = (u8)sighted_tile->count;
				while (count) {
					SightTile* test_tile = sighted_tile->tiles[--count];
					u32 rel_tile_pos = cur_tile_pos + test_tile->w + (g_map_tile_size->width * test_tile->h);
					if (!(rel_flags & test_tile->visibility_mask)
						&& bw::tile_is_equal_or_lower(g_temp_height, rel_tile_pos)
						&& (!(g_height_table[rel_tile_pos] == g_height_table[cur_tile_pos] && test_tile->visibility_mask & 0x1000000))) {
						refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
						sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
						break;
					}
				}
			}
			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall ground_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_vals, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_vals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_vals->revealed_count;

		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
			sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sight_vals->masked_count;
		while (masked_count) {
			sighted_tile->visibility_mask = 0xFF;
			u32 count = (u8)sighted_tile->count;
			while (count) {
				SightTile* test_tile = sighted_tile->tiles[--count];
				u32 rel_tile_pos = cur_tile_pos + test_tile->w + (g_map_tile_size->width * test_tile->h);

				if (!(rel_flags & test_tile->visibility_mask)
				&&    bw::tile_is_equal_or_lower(g_temp_height, rel_tile_pos)
				&& (!(g_height_table[rel_tile_pos] == g_height_table[cur_tile_pos] && test_tile->visibility_mask & 0x1000000)))
				{
					refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
					sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
					break;
				}
			}
			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall air_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_values->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_values->revealed_count;
		while (revealed_count) {
			if (tile_x + sighted_tile->w < g_map_tile_size->width && tile_y + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
				sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			}
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sight_values->masked_count;
		while (masked_count) {
			if (tile_x + sighted_tile->w < g_map_tile_size->width && tile_y + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
				current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			}
			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall air_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_values->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_values->revealed_count;
		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 10);
			sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sight_values->masked_count;
		while (masked_count) {
			refresh_vision_timers(g_map_tile_size->width * (tile_y + sighted_tile->h) + tile_x + sighted_tile->w, rel_flags, 10);
			current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall hit_sight_cb_edge(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_values->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_values->revealed_count;
		while (revealed_count) {
			if (tile_x + sighted_tile->w < g_map_tile_size->width && tile_y + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 18);
				sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			}
			++sighted_tile;
			--revealed_count;
		}
	}

	void __fastcall hit_sight_cb_inside(u32 tile_x, u32 tile_y, SightRange* sight_values, ActiveTile* current_active_tile, u32 rel_flags, u32 new_vision_flags) {
		SightTile* sighted_tile = sight_values->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tile_y + tile_x;
		u32 revealed_count = sight_values->revealed_count;
		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, rel_flags, 18);
			sighted_tile->visibility_mask = current_active_tile[sighted_tile->relative_tile].flags &= new_vision_flags;
			++sighted_tile;
			--revealed_count;
		}
	}

	void update_fog() {
		if (*g_multiplayer_mode) {
			*g_cheat_state &= ~(CheatFlags::BlackSheepWall + CheatFlags::WarAintWhatItUsedToBe);
		}
		if (!((*g_game_layer).has_been_refreshed & 1)) {
			(*g_game_layer).unk |= 1;
		}
		if (!(*g_cheat_state & (CheatFlags::BlackSheepWall + CheatFlags::WarAintWhatItUsedToBe))) {
			helper::refresh_layer_5();
			s32 i_0 = g_map_tile_size->width * g_map_tile_size->height - 1;
			ActiveTile* active_tile = *g_active_tile_array;
			do {
				s32 player = 7;
				u8 mask = 0xFF;
				do {
					bool revealed = vision_timers[i_0][player];
					mask ^= revealed << player;
					vision_timers[i_0][player] -= revealed;
				} while (--player >= 0);
				active_tile[i_0].visibility_flags = mask;
			} while (--i_0 >= 0);
		} else if (!(*g_cheat_state & CheatFlags::WarAintWhatItUsedToBe)) {
			u32 i_0 = g_map_tile_size->width * g_map_tile_size->height;
			u32* vision_timers = (u32*)&vision_timers[i_0];
			i_0 >>= 2;
			do {
				--vision_timers;
				*vision_timers = 0;
			} while (--i_0);
		}
	}

	void reveal_sight_at_location(u32 sight_range, u32 vision_flags, s32 pos_x, s32 pos_y, bool is_air_unit) {
		if (vision_flags <= 0xFF && !(*g_cheat_state & CheatFlags::BlackSheepWall)) {
			sight_callback_t** callback = g_sight_callbacks->inside;
			if (sight_range) {
				if (is_air_unit) {
					++callback; //air_sight
				} else {
					g_temp_height = get_ground_height_at_px(pos_y, pos_x);
				}
			} else {
				++sight_range;
				callback += 2; // hit_sight
			}

			SightRange* sight_vals = &g_sight_ranges[sight_range];
			s32 tile_sight_radius_x = sight_vals->tile_sight_width >> 1;
			s32 tile_sight_radius_y = sight_vals->tile_sight_height >> 1;
			pos_x >>= 5;
			pos_y >>= 5;
			if (pos_x - tile_sight_radius_x < 0
				|| pos_x + tile_sight_radius_x >= g_map_tile_size->width
				|| pos_y - tile_sight_radius_y < 0
				|| pos_y + tile_sight_radius_y >= g_map_tile_size->height) {
				callback += 3; // g_sight_callbacks->edge
			}

			ActiveTile* active_tile = *g_active_tile_array;
			(*callback)((u32)pos_x, (u32)pos_y, sight_vals, &active_tile[g_map_tile_size->width * pos_y + pos_x], vision_flags, ~vision_flags & ~(vision_flags << 8));
		}
	}

	s32 get_ground_height_at_px(s32 offset_y, s32 offset_x) {
		u32 grid_pos = g_map_tile_size->width * (offset_y >> 5) + (offset_x >> 5);
		u32 tile_id = (u16)g_zerg_cagra_tile_data->tile[grid_pos];

		if (!tile_id) {
			tile_id = (u16)g_mtxm_handle->tile[grid_pos];
		}
		u32 megatile_id = (u16)(g_cv5_file_handle->group[(tile_id >> 4) & 0xFFF].tiles[tile_id & 0xF]);
		vf4 minitile = g_vf4_file_handle->tile[megatile_id][4 * ((offset_y >> 3) & 3) + ((offset_x >> 3) & 3)];
		return ((minitile.flags >> 1) & 0x3) + minitile.xHeight;
	}
}