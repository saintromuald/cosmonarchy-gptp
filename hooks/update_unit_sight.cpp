#include "update_unit_sight.h"

namespace hooks {
	s32 tmpHeight;

	void __fastcall gndSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newvision_flags) {
		SightTile* sighted_tile = sightVals->tile;

		if (sightVals->always9) {
			u32 i_10 = sightVals->always9;

			do {
				sighted_tile->visibility_mask = 255;

				if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
					ActiveTile* mask_data = maskData + sighted_tile->relative_tile;
					mask_data->vision_flags &= newvision_flags;
					sighted_tile->visibility_mask = mask_data->vision_flags;
				}

				++sighted_tile;
			} while (--i_10);
		}

		s32 cur_height = tmpHeight;
		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			sighted_tile->visibility_mask = 255;

			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				u32 i_12 = (u8)sighted_tile->count;

				while (i_12) {
					SightTile* test_tile = sighted_tile->tiles[--i_12];

					if (scbw::tile_is_equal_or_lower(cur_height, tileY + test_tile->h, tileX + test_tile->w)
					&& !(relFlags & test_tile->visibility_mask)) {
						ActiveTile* mask_data = maskData + sighted_tile->relative_tile;
						mask_data->vision_flags &= newvision_flags;
						sighted_tile->visibility_mask = mask_data->vision_flags;
						break;
					}
				}
			}

			++sighted_tile;
			--tile_count;
		}
	}

	void __fastcall gndSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newvision_flags) {
		SightTile* sighted_tile = sightVals->tile;

		if (sightVals->always9) {
			u32 i_10 = sightVals->always9;

			do {
				ActiveTile* mask_data = maskData + sighted_tile->relative_tile;
				mask_data->vision_flags &= newvision_flags;
				sighted_tile->visibility_mask = mask_data->vision_flags;
				++sighted_tile;
			} while (--i_10);
		}

		s32 cur_height = tmpHeight;
		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			sighted_tile->visibility_mask = 255;
			u32 i_12 = (u8)sighted_tile->count;

			while (i_12) {
				SightTile* test_tile = sighted_tile->tiles[--i_12];

				if (scbw::tile_is_equal_or_lower(cur_height, tileY + test_tile->h, tileX + test_tile->w)
				&& !(relFlags & test_tile->visibility_mask)) {
					ActiveTile* mask_data = maskData + sighted_tile->relative_tile;
					mask_data->vision_flags &= newvision_flags;
					sighted_tile->visibility_mask = mask_data->vision_flags;
					break;
				}
			}

			++sighted_tile;
			--tile_count;
		}
	}

	void reveal_sight_at_location(u32 sightRange, u32 vision_flags, s32 ptX, s32 ptY, bool isAirUnit) {
		if (vision_flags <= 0xFF) {
			SightRange* sight_vals = &g_sight_values[sightRange];
			u32 new_vision_flags = ~vision_flags & ~(vision_flags << 8);
			sight_callback_t** callback = g_sight_callbacks->ground;

			if (isAirUnit) {
				++callback;
			} else {
				tmpHeight = get_ground_height_at_px(ptY, ptX);
			}

			s32 tileX = ptX >> 5;
			s32 tileSightXRadius = sight_vals->tile_sight_width >> 1;
			s32 tileY = ptY >> 5;
			s32 tileSightYRadius = sight_vals->tile_sight_height >> 1;

			if (tileX - tileSightXRadius < 0
			|| tileX + tileSightXRadius >= g_map_tile_size->width
			|| tileY - tileSightYRadius < 0
			|| tileY + tileSightYRadius >= g_map_tile_size->height) {
				callback += 2;//g_sight_callbacks->edge
			}

			ActiveTile* active_tile = *activeTileArray;
			(*callback)((u32)tileX, (u32)tileY, sight_vals, &active_tile[tileX + tileY * g_map_tile_size->width], vision_flags, new_vision_flags);
		}
	}

	s32 get_ground_height_at_px(s32 offsetY, s32 offsetX) {
		u32 grid_pos = g_map_tile_size->width * (offsetY >> 5) + (offsetX >> 5);
		u32 tile_id = (u16)(*zergCreepTileData)->tile[grid_pos];

		if (!tile_id)
			tile_id = (u16)(*MTXMHandle)->tile[grid_pos];

		u32 megatile_id = (u16)((*cv5FileHandle)->group[(tile_id >> 4) & 0x7FF].tiles[tile_id & 0xF]);
		vf4 minitile = (*vf4FileHandle)->tile[megatile_id][4 * ((offsetY >> 3) & 3) + ((offsetX >> 3) & 3)];
		return ((minitile.flags >> 1) & 0x3) + minitile.xHeight;
	}
}