#pragma once
#include "../../systems/display/display.h"
#include "../../BW/bwdata.h"

namespace hooks {

// The tile pipeline: GPTP owns tile loading, change detection, and rendering.
// The C++ replacements absorb the tile patches of vx4ex.qdp (u32 minitile refs,
// ".vx4ex" tilesets), aise.qdp (extended megatile ids, relocated creep-border
// flag), and the tile-group block that used to live in initialize.cpp.

void blit_minitile(s32 offset, vx4 ref);                                                                 // 0040C3B0
void draw_megatile(s32 tile_x, s32 tile_y, u16 megatile, s32 offset);                                    // 0049B9F0
void set_map_tiles();                                                                                    // 004BCEA0
void update_single_tile(s32 tile_x, s32 tile_y, u32 mode, u16 tile_id, const void* refresh_rect);        // 0047E2D0
void update_tile_row(u16* megatiles, const u16* tile_ids, u32* tile_flags, u32 tile_y, s32 offset);                // 0049C620
void progress_creep_disappearance();                                                                    // 0049C4C0

// Vanilla helpers the tile subsystem calls (naked wrappers in tiles_inject.cpp).
void refresh_map_area(const void* rect);                                                                 // 0047D920, rect travels in EAX
void update_creep_groups_near(s32 tile_x, s32 tile_y);                                                   // 0047DC10, callee-clean

void inject_tiles_hooks();

} // hooks