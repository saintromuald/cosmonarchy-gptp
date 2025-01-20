#include <BW/extender/TileHeight_extender.h>

namespace bw {

vf4* get_minitile(u32 grid_pos) {
	u32 tile_id = (u16)(*g_zerg_cagra_tile_data)->tile[grid_pos];

	if (!tile_id) {
		tile_id = (u16)(*g_MTXM_handle)->tile[grid_pos];
	}

	u32 megatile_id = (u16)((*g_cv5_file_handle)->group[(tile_id >> 4) & 0xFFF].tiles[tile_id & 0xF]);
	return (vf4*)(*g_vf4_file_handle)->tile[megatile_id + 1];
}

void init_weapon_tile_heights() {
	u32 map_size = g_map_tile_size->width * g_map_tile_size->height;
	g_base_height = 127;

	for (u32 tile = 0; tile < map_size; ++tile) {
		vf4* minitile = get_minitile(tile);
		s32 i0 = 0;
		s32 height = 0;

		do {
			if ((--minitile)->flags & 1) {
				height += ((minitile->flags >> 1) & 0x3) + minitile->xHeight;
			}
		} while (++i0 < 16);

		s32 i1 = height < 0 ? -i0 : i0;
		height = (height + (i1 >> 1)) / i0;

		if (height > 127) {
			height = 127;
		}

		g_height_table[tile] = height;

		if (height < g_base_height) {
			g_base_height = height;
		}
	}
}

bool tile_is_equal_or_lower(s32 height, u32 offset) {
	vf4* minitile = get_minitile(offset);

	u32 i0 = 16;
	u32 i1 = 0;
	do {
		--minitile;

		if (((minitile->flags >> 1) & 0x3) + minitile->xHeight <= height) {
			++i1;
		}
	} while (i1 < 5 && --i0);

	return i1 >= 5;
}

}