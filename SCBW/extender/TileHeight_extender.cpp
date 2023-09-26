#include <SCBW/extender/TileHeight_extender.h>

s32 base_height;
s8 xHeight_table[256*256];

namespace scbw {
	vf4* get_minitile(u32 gridPos) {
		u32 tile_id = (u16)(*zergCreepTileData)->tile[gridPos];

		if (!tile_id)
			tile_id = (u16)(*MTXMHandle)->tile[gridPos];

		u32 megatile_id = (u16)((*cv5FileHandle)->group[(tile_id >> 4) & 0x7FF].tiles[tile_id & 0xF]);
		return (vf4*)(*vf4FileHandle)->tile[megatile_id + 1];
	}

	void weapon_tile_heights() {
		u32 map_size = g_map_tile_size->width * g_map_tile_size->height;
		base_height = 127;

		for (u32 tile = 0; tile < map_size; ++tile) {
			vf4* minitile = get_minitile(tile);
			u32 i_0 = 0;
			s32 height = 0;

			do {
				--minitile;
				if (minitile->flags & 1) {
					height += ((minitile->flags >> 1) & 0x3) + minitile->xHeight;
				}
			} while (++i_0 < 16);

			height = (height + (i_0 >> 1)) / i_0;
			xHeight_table[tile] = height;

			if (height < base_height)
				base_height = height;
		}
	}

	bool tile_is_equal_or_lower(s32 height, u32 offsetY, u32 offsetX) {
		vf4* minitile = get_minitile(g_map_tile_size->width * offsetY + offsetX);
		u32 i_0 = 16;
		u32 i_1 = 0;

		do {
			--minitile;

			if (((minitile->flags >> 1) & 0x3) + minitile->xHeight <= height) {
				++i_1;
			}
		} while (i_1 < 5 && --i_0);

		return i_1 >= 5;
	}
}