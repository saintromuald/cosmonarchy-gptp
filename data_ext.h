#pragma once
#include "SCBW/api.h"

// TODO: move unit ext here

struct TileExt {
	u8 creep_owners;
};

void reset_ext_variables();
void update_ext_variables();

TileExt* get_tile_ext_at_tile(u32 tile_x, u32 tile_y);
TileExt* get_tile_ext_at_coords(u32 x, u32 y);