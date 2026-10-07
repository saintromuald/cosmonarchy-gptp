#pragma once
#include "../../systems/display/display.h"

namespace hooks {

s32 __cdecl begin_frame();                                             // 0041D710
u8 __stdcall rect_is_dirty(s32 left, s32 top, s32 right, s32 bottom);  // 0041DE20
void __cdecl mark_rect(s32 left, s32 top, s32 right, s32 bottom);      // 0041E0D0
u32 __cdecl clip_rect_to_world(s32* rect);                             // 0045CC90
void __cdecl mark_rect_cells(s32* rect);                               // 0042D280
void __stdcall draw_box(s32 x, s32 y, s32 box_w, s32 box_h);           // 004E1D20
u32 __cdecl object_is_dirty(const u8* object);                         // 00497000
void __cdecl expire_text_line(u32 index);                              // 0048CB80
void __cdecl flush_region();                                           // 0041E000
void __cdecl present_frame();                                          // 0041E280
void __cdecl refresh_world_cells();                                    // 004BCDC0
void __cdecl init_game_layer();                                        // 004BD630

// Re-resolves the replay/witness role and reapplies the display geometry
// derived from it (zoom floor/world extent, sprite clip, game layer).
void refresh_display_role();

// Is there anything for the next present_frame to draw: a pointer move, a
// window repaint, a dialog asking for one, a layer asked to run, or a cell the
// world marked. Bit 5 of a layer's flags is left out - it means the layer runs
// on every present, not that it has something new.
bool has_pending_draw();

void inject_grid_hooks();

} // hooks
