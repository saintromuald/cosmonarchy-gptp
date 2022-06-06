#include "data_ext.h"

static TileExt tiles_ext[256][256];

void reset_ext_variables() {
	memset(tiles_ext, 0, sizeof(tiles_ext));
}

void update_ext_variables() {
	// Tiles
	for (u32 y = 0; y < map_tile_size->height; ++y) {
		for (u32 x = 0; x < map_tile_size->width; ++x) {
			TileExt* tile_ext = get_tile_ext_at_tile(x, y);
			tile_ext->creep_owners = 0;
		}
	}
}

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y) {
	return &tiles_ext[tile_y][tile_x];
}

TileExt* get_tile_ext_at_coords(u32 x, u32 y) {
	return &tiles_ext[y / 32][x / 32];
}

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player) {
	u32 allied_bits = 0;
	for (u32 i = 0; i < PlayerLimit; ++i) {
		if (scbw::is_allied_to(i, player)) {
			allied_bits |= 1 << i;
		}
	}
	return !tile_ext->creep_owners || tile_ext->creep_owners & allied_bits;
}