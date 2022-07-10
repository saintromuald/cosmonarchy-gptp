#include "data_ext.h"

TileExt tile_ext_array[256][256];
SpriteExt sprite_ext_array[SPRITE_ARRAY_LENGTH];

void init_ext_variables() {
	memset(tile_ext_array, 0, sizeof(tile_ext_array));
	memset(sprite_ext_array, 0, sizeof(sprite_ext_array));
}

void update_ext_variables() {
	// Tiles
	for (u32 y = 0; y < g_map_tile_size->height; ++y) {
		for (u32 x = 0; x < g_map_tile_size->width; ++x) {
			TileExt* tile_ext = get_tile_ext_at_tile(x, y);
			tile_ext->creep_owners = 0;
		}
	}

	// Sprites
	for (u32 i = 0; i < SPRITE_ARRAY_LENGTH; ++i) {

	}
}

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y) {
	return &tile_ext_array[tile_y][tile_x];
}

TileExt* get_tile_ext_at_coords(u32 x, u32 y) {
	return &tile_ext_array[y / 32][x / 32];
}

SpriteExt* get_sprite_ext(Sprite* sprite) {
	s32 index = sprite - g_sprite_table;
	assert(index >= 0 && index <= SPRITE_ARRAY_LENGTH);
	return &sprite_ext_array[index];
}

void remove_sprite_ext(Sprite* sprite) {
	SpriteExt* sprite_ext = get_sprite_ext(sprite);
	*sprite_ext = {};
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