#include "data_ext.h"

static TileExt tiles_ext[256][256];

void reset_ext_variables() {
	memset(tiles_ext, 0, sizeof(tiles_ext));
}

void update_ext_variables() {
	// Tiles
	for (u32 y = 0; y < mapTileSize->height; ++y) {
		for (u32 x = 0; x < mapTileSize->width; ++x) {
			ActiveTile* active_tile = scbw::get_active_tile_at_tile(x, y);
			if (!active_tile->has_creep && !active_tile->temporary_creep) {
				TileExt* tile_ext = get_tile_ext_at_tile(x, y);
				tile_ext->creep_owners = 0;
			}
		}
	}
}

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y) {
	return &tiles_ext[tile_y][tile_x];
}

TileExt* get_tile_ext_at_coords(u32 x, u32 y) {
	return &tiles_ext[y / 32][x / 32];
}