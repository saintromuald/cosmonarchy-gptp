#include "update_unit_sight.h"
#include "SCBW/ExtendSightLimit.h"

u8 vision_timers[8][256 * 256];

namespace hooks {
	s32 tmpHeight;
	SightCallbacks sightCallbacks[] = { gndSightCB_Inside, airSightCB_Inside, gndSightCB_Edge, airSightCB_Edge };

	void __fastcall gndSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;

		if (sightVals->always9) {
			u32 i_10 = sightVals->always9;

			do {
				sighted_tile->visibility_mask = 0xFF;

				if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
					ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
					mask_data->tileFlags &= newVisionFlags;
					sighted_tile->visibility_mask = mask_data->tileFlags;
				}

				++sighted_tile;
			} while (--i_10);
		}

		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			sighted_tile->visibility_mask = 0xFF;

			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				u32 i_12 = (u8)sighted_tile->count;

				while (i_12) {
					SightTile* test_tile = sighted_tile->tiles[--i_12];

					if (!(relFlags & test_tile->visibility_mask)
						&& scbw::tile_is_equal_or_lower(tmpHeight, tileY + test_tile->h, tileX + test_tile->w)) {
						u32 test_tile_pos = g_map_tile_size->width * (tileY + test_tile->h) + tileX + test_tile->w;

						if (xHeight_table[g_map_tile_size->width * tileY + tileX] == xHeight_table[test_tile_pos]
							&& test_tile->visibility_mask & 0x1000000) {
							break;
						}

						refresh_vision_timers(test_tile_pos, relFlags);
						ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
						mask_data->tileFlags &= newVisionFlags;
						sighted_tile->visibility_mask = mask_data->tileFlags;
						break;
					}
				}
			}

			++sighted_tile;
			--tile_count;
		}
	}

	void __fastcall gndSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;

		if (sightVals->always9) {
			u32 i_10 = sightVals->always9;

			do {
				ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
				mask_data->tileFlags &= newVisionFlags;
				sighted_tile->visibility_mask = mask_data->tileFlags;
				++sighted_tile;
			} while (--i_10);
		}

		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			sighted_tile->visibility_mask = 0xFF;
			u32 i_12 = (u8)sighted_tile->count;

			while (i_12) {
				SightTile* test_tile = sighted_tile->tiles[--i_12];

				if (!(relFlags & test_tile->visibility_mask)
					&& scbw::tile_is_equal_or_lower(tmpHeight, tileY + test_tile->h, tileX + test_tile->w)) {
					u32 test_tile_pos = g_map_tile_size->width * (tileY + test_tile->h) + tileX + test_tile->w;

					if (xHeight_table[g_map_tile_size->width * tileY + tileX] == xHeight_table[test_tile_pos]
						&& test_tile->visibility_mask & 0x1000000) {
						break;
					}

					refresh_vision_timers(test_tile_pos, relFlags);
					ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
					mask_data->tileFlags &= newVisionFlags;
					sighted_tile->visibility_mask = mask_data->tileFlags;
					break;
				}
			}

			++sighted_tile;
			--tile_count;
		}
	}

	void __fastcall airSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			u32 sighted_tile_x = tileX + sighted_tile->w;
			u32 sighted_tile_y = tileY + sighted_tile->h;

			if (sighted_tile_x < g_map_tile_size->width && sighted_tile_y < g_map_tile_size->height) {
				refresh_vision_timers(g_map_tile_size->width * sighted_tile_y + sighted_tile_x, relFlags);
				ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
				mask_data->tileFlags &= newVisionFlags;
			}

			++sighted_tile;
			--tile_count;
		}
	}

	void __fastcall airSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 tile_count = sightVals->tile_count;

		while (tile_count) {
			refresh_vision_timers(g_map_tile_size->width * (tileY + sighted_tile->h) + tileX + sighted_tile->w, relFlags);
			ActiveTileDF* mask_data = (ActiveTileDF*)maskData + sighted_tile->relative_tile;
			mask_data->tileFlags &= newVisionFlags;
			++sighted_tile;
			--tile_count;
		}
	}

	void refresh_vision_timers(u32 testTilePos, u8 visionFlags) {
		u8 player = 7;

		do {
			if (visionFlags & (1 << player)) {
				vision_timers[player][testTilePos] = 10;
			}

			if (!player)
				break;

			--player;
		} while (true);
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
			ActiveTileDF* active_tile =	(ActiveTileDF*)*activeTileArray;
			u32 i_0 = g_map_tile_size->width * g_map_tile_size->height;

			do {
				u8 mask = 0;
				u8 player = 7;

				do {
					if (!vision_timers[player][i_0]) {
						mask |= 1 << player;
					}
					else {
						--vision_timers[player][i_0];
					}

					if (!player)
						break;

					--player;
				} while (true);

				active_tile[i_0].revealedMask |= mask;

				if (!i_0)
					break;

				--i_0;
			} while (true);
		}
	}

	void reveal_sight_at_location(u32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit) {
		if (visionFlags <= 0xFF) {
			SightRange* sight_vals = &g_sight_ranges[sightRange];
			u32 new_vision_flags = ~visionFlags & ~(visionFlags << 8);
			sight_callback_t** callback = sightCallbacks->ground;

			if (isAirUnit) {
				++callback;
			}
			else {
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
				callback += 2;//sightCallbacks->edge
			}

			ActiveTile* active_tile = *activeTileArray;
			(*callback)((u32)tileX, (u32)tileY, sight_vals, &active_tile[tileX + tileY * g_map_tile_size->width], visionFlags, new_vision_flags);
		}
	}

	s32 get_ground_height_at_px(s32 offsetY, s32 offsetX) {
		u32 grid_pos = g_map_tile_size->width * (offsetY >> 5) + (offsetX >> 5);
		u32 tile_id = (u16)(*zergCreepTileData)->tile[grid_pos];

		if (!tile_id)
			tile_id = (u16)(*MTXMHandle)->tile[grid_pos];

		u32 megatile_id = (u16)((*cv5FileHandle)->group[(tile_id >> 4) & 0xFFF].tiles[tile_id & 0xF]);
		vf4 minitile = (*vf4FileHandle)->tile[megatile_id][4 * ((offsetY >> 3) & 3) + ((offsetX >> 3) & 3)];
		return ((minitile.flags >> 1) & 0x3) + minitile.xHeight;
	}
}