#include "update_unit_sight.h"
#include "BW/ExtendSightLimit.h"

s32 tmpHeight;
u8 vision_timers[256*256][8];

namespace hooks {
	SightCallbacks sightCallbacks[] = { gndSightCB_Inside, airSightCB_Inside, hitSightCB_Inside, gndSightCB_Edge, airSightCB_Edge, hitSightCB_Edge };

	inline void refresh_vision_timers(u32 testTilePos, u8 visionFlags, u8 timer) {
		s32 player = 7;
		u8 player_flag = 0x80;
		u8* cur_vision_timers = &vision_timers[testTilePos][0];

		do {
			if (visionFlags & player_flag && *(cur_vision_timers + player) < timer) {
				*(cur_vision_timers + player) = timer;
			}
			player_flag >>= 1;
		} while (--player >= 0);
	}

	void __fastcall gndSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			sighted_tile->visibility_mask = 0xFF;

			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
				sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			}

			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sightVals->masked_count;

		while (masked_count) {
			sighted_tile->visibility_mask = 0xFF;

			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				u32 count = (u8)sighted_tile->count;

				while (count) {
					SightTile* test_tile = sighted_tile->tiles[--count];
					u32 rel_tile_pos = cur_tile_pos + test_tile->w + (g_map_tile_size->width * test_tile->h);

					if (!(relFlags & test_tile->visibility_mask)
						&& bw::tile_is_equal_or_lower(tmpHeight, rel_tile_pos)
						&& (!(g_height_table[rel_tile_pos] == g_height_table[cur_tile_pos] && test_tile->visibility_mask & 0x1000000))) {
						refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
						sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
						break;
					}
				}
			}

			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall gndSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
			sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sightVals->masked_count;

		while (masked_count) {
			sighted_tile->visibility_mask = 0xFF;
			u32 count = (u8)sighted_tile->count;

			while (count) {
				SightTile* test_tile = sighted_tile->tiles[--count];
				u32 rel_tile_pos = cur_tile_pos + test_tile->w + (g_map_tile_size->width * test_tile->h);

				if (!(relFlags & test_tile->visibility_mask)
				&&    bw::tile_is_equal_or_lower(tmpHeight, rel_tile_pos)
				&& (!(g_height_table[rel_tile_pos] == g_height_table[cur_tile_pos] && test_tile->visibility_mask & 0x1000000)))
				{
					refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
					sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
					break;
				}
			}

			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall airSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
				sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			}

			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sightVals->masked_count;

		while (masked_count) {
			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
				curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			}

			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall airSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 10);
			sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			++sighted_tile;
			--revealed_count;
		}

		u32 masked_count = sightVals->masked_count;

		while (masked_count) {
			refresh_vision_timers(g_map_tile_size->width * (tileY + sighted_tile->h) + tileX + sighted_tile->w, relFlags, 10);
			curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			++sighted_tile;
			--masked_count;
		}
	}

	void __fastcall hitSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			if (tileX + sighted_tile->w < g_map_tile_size->width && tileY + sighted_tile->h < g_map_tile_size->height) {
				refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 18);
				sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			}

			++sighted_tile;
			--revealed_count;
		}
	}

	void __fastcall hitSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* curActiveTile, u32 relFlags, u32 newVisionFlags) {
		SightTile* sighted_tile = sightVals->tile;
		u32 cur_tile_pos = g_map_tile_size->width * tileY + tileX;
		u32 revealed_count = sightVals->revealed_count;

		while (revealed_count) {
			refresh_vision_timers(cur_tile_pos + sighted_tile->relative_tile, relFlags, 18);
			sighted_tile->visibility_mask = curActiveTile[sighted_tile->relative_tile].flags &= newVisionFlags;
			++sighted_tile;
			--revealed_count;
		}
	}

	void update_fog() {
		if (bw::get_game_kind() == GameKind::DebugFog) {
			return;
		}

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
			/*__asm {
				MOVZX	EAX, word ptr ds:[0x57F1D6]//g_map_tile_size->height
				MOVZX	EDX, word ptr ds:[0x57F1D4]//g_map_tile_size->width
				IMUL	EDX, EAX
				DEC		EDX
				MOV		EAX, [g_active_tile_array]
				LEA		EDI, [EAX + EDX * 4]//activeTile
				LEA		ESI, [EDX * 8 + visionTimers + 7]//visionTimers
				ALIGN 4

				off_0:
				MOV		ECX, 7//player = 7
				OR		BH, -1//mask = 0xFF
				ALIGN 4

				off_1:
				CMP		[ESI], 0//visionTimers[i_0][player] != 0
				SETNE	BL//revealed
				MOV		AL, BL
				SHL		AL, CL
				SUB		[ESI],BL//visionTimers[i_0][player] -= revealed
				XOR		BH, AL//mask ^= revealed << player
				DEC		ESI//visionTimers
				DEC		ECX//--player
				JNS		offset off_1
				MOV		[EDI], BH//active_tile[i_0].visibility_flags = mask
				SUB		EDI, 4
				DEC		EDX//--i_0
				JNS		offset off_0
			}*/
		}
		else if (!(*g_cheat_state & CheatFlags::WarAintWhatItUsedToBe)) {
			u32 i_0 = g_map_tile_size->width * g_map_tile_size->height;
			u32* vision_timers = (u32*)&vision_timers[i_0];
			i_0 >>= 2;

			do {
				--vision_timers;
				*vision_timers = 0;
			} while (--i_0);
		}
	}

	void reveal_sight_at_location(u32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit) {
		if (visionFlags <= 0xFF && !(*g_cheat_state & CheatFlags::BlackSheepWall)) {
			sight_callback_t** callback = sightCallbacks->inside;

			if (sightRange) {
				if (isAirUnit) {
					++callback;//airSight
				}
				else {
					tmpHeight = get_ground_height_at_px(ptY, ptX);
				}
			} else {
				++sightRange;
				callback += 2;//hitSight
			}

			SightRange* sight_vals = &g_sight_ranges[sightRange];
			s32 tile_sight_radius_x = sight_vals->tile_sight_width >> 1;
			s32 tile_sight_radius_y = sight_vals->tile_sight_height >> 1;
			ptX >>= 5;
			ptY >>= 5;

			if (ptX - tile_sight_radius_x < 0
				|| ptX + tile_sight_radius_x >= g_map_tile_size->width
				|| ptY - tile_sight_radius_y < 0
				|| ptY + tile_sight_radius_y >= g_map_tile_size->height) {
				callback += 3;//sightCallbacks->edge
			}

			ActiveTile* active_tile = *g_active_tile_array;
			(*callback)((u32)ptX, (u32)ptY, sight_vals, &active_tile[g_map_tile_size->width * ptY + ptX], visionFlags, ~visionFlags & ~(visionFlags << 8));
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