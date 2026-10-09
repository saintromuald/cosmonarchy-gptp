#pragma once
#include "../../systems/display/display.h"

namespace display {

// World-phase dirty-cell storage: the world grid while zoom presents, the
// surface grid otherwise (identical lattices at 100%). The fog walker and the
// starfield read marks through this view.
u8* world_phase_cells();
s32 world_phase_stride();

// World-phase lattice extents, in cells.
s32 world_phase_cols();
s32 world_phase_rows();

// True while the zoom transform presents (world magnified, transitioning, or
// anchored) and the game layer is running.
bool world_grid_active();

// Fills the world grid; full-world refreshes (refresh_screen, shape redraws)
// fill this alongside the surface grid so the world phase re-blits and the
// propagation rescales.
void fill_world_grid(u8 value);

// Re-resolves the replay/witness role and reapplies the display geometry
// derived from it (zoom floor/world extent, sprite clip, game layer).
void refresh_display_role();

// Is there anything for the next present_frame to draw: a pointer move, a
// window repaint, a dialog asking for one, a layer asked to run, or a cell the
// world marked. LayerFlags::AlwaysUpdate is left out - it means the layer runs
// on every present, not that it has something new.
bool has_pending_draw();

} // display

namespace hooks {

s32 __cdecl begin_frame();                                             // 0041D710
u8 __stdcall rect_is_dirty(s32 left, s32 top, s32 right, s32 bottom);  // 0041DE20
void __cdecl mark_rect(s32 left, s32 top, s32 right, s32 bottom);      // 0041E0D0

// Marks the world-phase grid over the rect (world pixels): the world grid while
// zoom presents, the surface grid otherwise, where world pixels are surface pixels.
// Fog changes and placement layer erases mark through this.
void mark_world_rect(s32 left, s32 top, s32 right, s32 bottom);

u32 __cdecl clip_rect_to_world(s32* rect);                             // 0045CC90
void __cdecl mark_rect_cells(s32* rect);                               // 0042D280
void __stdcall draw_box(s32 x, s32 y, s32 box_w, s32 box_h);           // 004E1D20
u32 __cdecl object_is_dirty(const u8* object);                         // 00497000
void __cdecl expire_text_line(u32 index);                              // 0048CB80
void __cdecl flush_region();                                           // 0041E000
void __cdecl present_frame();                                          // 0041E280
void __cdecl refresh_world_cells();                                    // 004BCDC0
void __cdecl init_game_layer();                                        // 004BD630

void inject_grid_hooks();

} // hooks