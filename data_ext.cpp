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
			TileExt* tile_ext = get_tile_ext_at_tile(x, y);

			// Creep reclamation check
			for (u32 player = 0; player < PlayerLimit; ++player) {
				bool player_is_owner = tile_ext->creep_owners & 1 << player;
				bool player_has_generator = tile_ext->creep_generators & 1 << player;
				if (player_is_owner && !player_has_generator) {
					bool has_allied_generator = false;
					bool has_enemy_generator = false;
					for (u32 other = 0; other < PlayerLimit; ++other) {
						bool other_has_generator = tile_ext->creep_generators & 1 << other;
						if (other_has_generator) {
							if (scbw::is_allied_to(other, player)) {
								has_allied_generator = true;
							} else {
								has_enemy_generator = true;
							}
						}
					}
					if (!has_allied_generator && has_enemy_generator) {
						tile_ext->creep_owners &= ~(1 << player);
					}
				}
			}

			// Reset generators temp
			tile_ext->creep_generators = 0;

			// Reset owners when no creep
			if (!active_tile->has_creep && !active_tile->temporary_creep) {
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