#pragma once
#include "SCBW/api.h"

// TODO: move unit ext here

struct TileExt {
	u8 creep_owners;
};

struct SpriteExt {
	bool is_fow_sprite;
};

extern TileExt tile_ext_array[256][256];
extern SpriteExt sprite_ext_array[SPRITE_ARRAY_LENGTH];

void init_ext_variables();
void update_ext_variables();

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y);
TileExt* get_tile_ext_at_coords(u32 x, u32 y);

SpriteExt* get_sprite_ext(Sprite* sprite);
void remove_sprite_ext(Sprite* sprite);

bool can_place_on_creep_tile(TileExt* tile_ext, u32 player);