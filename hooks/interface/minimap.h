#pragma once
#include "BW/bwdata.h"

s8 get_minimap_color(u8 player);

namespace hooks {
    int __fastcall minimapVisionUpdate_256();																// 004A3A40
    int __fastcall minimapVisionUpdate_128();																// 004A3B30
    int __fastcall minimapVisionUpdate_64();																// 004A3C00
    int __fastcall minimapVisionUpdate_staircase();
    void minimapSurfaceUpdate_256();															// 004A41B0
    void minimapSurfaceUpdate_128();															// 004A4240
    void minimapSurfaceUpdate_64();																// 004A42B0

    void __cdecl set_map_size_constants();                                                      // 004A4400

    void minimap_ping(s32 pos_x, s32 pos_y, u8 color);                                            // 004A34C0
    void __fastcall minimap_image_update(BinDlg* minimap, DialogEvent* event, u32 user, void* draw_context);	// 004A3EE0
    void __fastcall minimap_overlay_draw(bw::Dialog* dialog, DialogEvent* event, u32 user, void* draw_ctx);  // 004A3570
    void draw_unit_box(s32 posX, u8 color, s32 posY, u32 sizeX, u32 sizeY, bool is_building);	// 004A3FD0
    void minimap_game_mouse_update(bw::Dialog* minimap);                                        // 004A4D20
    u16 __cdecl minimap_cursor_to_world(u16* x, u16* y);										// 004A3D70
    const RECT& refresh_minimap_rect(BinDlg* minimap);
    void minimap_timer();																		// 004A4E00

    void inject_minimap_hooks();
}