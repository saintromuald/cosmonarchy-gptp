#pragma once
#include "../../systems/display/display.h"

namespace hooks {

u32 __cdecl clip_tile_rect(s32* tile_x, s32* tile_y, s32* out_width, s32* out_height);  // 0049B8D0
void __cdecl blit_overlay(const u8* record, s32 offset);                                // 0040AAE0
void __cdecl draw_tile_row(const u16* megatiles, u32 tile_y, u32 tile_x);               // 0049BE20
u32 __cdecl draw_tile_column(const u16* megatiles, u32 tile_x, u32 tile_y);            // 0049BD40
void __cdecl draw_tile_rect(s32 tile_x, s32 tile_y, s32 width, s32 height);             // 0049BC20
void __cdecl draw_all_tile_rows();                                                      // 0049BF20
void __cdecl redraw_terrain();                                                          // 0049C780
u32 __cdecl scroll_right(s32 amount);                                                   // 0049C0C0
u32 __cdecl scroll_left(s32 amount);                                                    // 0049C1A0
u32 __cdecl scroll_down(s32 amount);                                                    // 0049C280
u32 __cdecl scroll_up(s32 amount);                                                      // 0049C360

void inject_terrain_hooks();

} // hooks
