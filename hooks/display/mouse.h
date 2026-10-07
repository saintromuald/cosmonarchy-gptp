#pragma once
#include "../../systems/display/display.h"
#include "../../BW/bwdata.h"

namespace hooks {

// Cursor position in world coordinates (camera offset). 004D13E0 reads this
// instead of g_mouse to determine cursor shape under zoom.
inline v2s32 g_world_cursor;

// Real OS focus state as of the last update_window_focus poll. cnc-ddraw eats
// WM_ACTIVATEAPP after the first delivery, so the game's own tracking (and its
// g_window_active flag) never sees later activations.
inline bool g_window_focused = true;

// Real screen-space cursor position captured while the window is unfocused.
inline v2s32 g_unfocused_cursor_pos = {};

// Buttons whose refocusing click was swallowed; their releases are swallowed too.
inline u32 g_refocus_swallowed_buttons = 0;

// Updates g_world_cursor from current mouse and camera coordinates; called per frame.
void update_world_cursor();

// Per-frame real OS focus poll (begin_frame). Undoes cnc-ddraw's refocus warp: in
// borderless fullscreen, mouse_lock warps the cursor to its stale tracked position
// on WM_ACTIVATEAPP, before the refocusing click is processed.
void update_window_focus();

// 004D1140: gameplay-area gate for clicks and cursor shape, in world coordinates.
u32 is_point_on_console_ui(s32 x, s32 y);

// Warps the OS cursor to a client-space point (screen-converted).
void warp_cursor(v2s32 surface_point);

void __cdecl send_mouse_move_event();                                                                             // 004D1AE0
void __cdecl mouse_button_up(u32 button_bit, u32 packed_position, InputProcsFunc* input_proc, u32 event_type);    // 004D1940
void __cdecl mouse_button_down(u32 button_bit, u32 packed_position, InputProcsFunc* input_proc, u32 event_type);  // 004D19C0
s32 __cdecl edge_scroll_direction(s32* out_x, s32* out_y);                                                        // 004D12A0
void __cdecl set_cursor_clip_rect();                                                                              // 004215E0

// Jump target from main_window_proc (004D24DC).
void window_proc_mouse_move(u32 packed_position);                                                                 // 004D24DC

void __cdecl update_cursor();           // 004D14D0
void __cdecl refresh_cursor();          // 004BE120
void __cdecl cursor_refresh();          // 004BE060
void __cdecl refresh_cursor_0();        // 004BE0B0
void show_os_cursor(u32 inactive);      // 004D1750

// Snaps the OS cursor and g_mouse to the surface center at process launch (004E087A) and game start (004D9975)
void center_cursor();

// Set during middle-drag to draw the pinned software cursor layer in present_frame.
inline bool g_middle_drag_cursor = false;

void begin_middle_drag_cursor(v2s32 grab_point);
void end_middle_drag_cursor();

void inject_mouse_hooks();

} // hooks
