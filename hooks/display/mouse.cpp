#include "mouse.h"
#include "grid.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../systems/display/display.h"
#include "../../systems/display/zoom.h"
#include "../../systems/display/cursor_cache.h"
#include "../interface/uifeatures/ui_features.h"

#include <algorithm>

namespace {

constexpr u32 CursorFrameMs = 100;
constexpr u32 MiddleDragCursorId = 9; // Drag.grp

v2s32 g_middle_drag_grab;

u16 clamp_cursor_x(u32 packed_position) {
    return u16(std::clamp(s32(s16(packed_position)), 0, s32(g_display.cursor_max_x())));
}

u16 clamp_cursor_y(u32 packed_position) {
    return u16(std::clamp(s32(s16(packed_position >> 16)), 0, s32(g_display.cursor_max_y())));
}

// Offers mouse event to dialog list in surface coords; unhandled events pass to input_proc in world coords.
void offer_mouse_event(DialogEvent* event, InputProcsFunc* input_proc) {
    if (helper::send_input_to_all_dialogs(event) != 0) {
        return;
    }
    if (!input_proc) {
        return;
    }

    const v2s32 world = g_zoom.surface_to_world_offset({ s32(event->current.x), s32(event->current.y) });
    event->current.x = u16(world.x);
    event->current.y = u16(world.y);
    input_proc(s32(event));
}

// Packs mouse event for dialog and game dispatch.
void dispatch_mouse_event(u32 packed_position, InputProcsFunc* input_proc, u32 event_type) {
    DialogEvent event{};
    event.number_order = DialogEventType(u16(event_type));
    event.current.x = clamp_cursor_x(packed_position);
    event.current.y = clamp_cursor_y(packed_position);

    g_mouse->x = s32(event.current.x);
    g_mouse->y = s32(event.current.y);

    offer_mouse_event(&event, input_proc);
}

} // namespace

namespace hooks {

// Warps the OS cursor to a client-space point (screen-converted).
void warp_cursor(v2s32 surface_point) {
    POINT screen_point = { surface_point.x, surface_point.y };
    if (g_main_window) {
        g_client_to_screen(g_main_window, &screen_point);
    }
    g_set_cursor_pos_iat(screen_point.x, screen_point.y);
}

// 004D1AE0: Dispatches mouse move when no buttons held; clears bit 0 (cursor in window).
void __cdecl send_mouse_move_event() {
    g_mouse_button_state &= 0xFFFE;

    if ((g_mouse_button_state & 2) != 0) {
        g_wants_redraw = 1;
    }

    DialogEvent event{};
    event.number_order = DialogEventType_MouseMove;
    event.current.x = u16(g_mouse->x);
    event.current.y = u16(g_mouse->y);

    offer_mouse_event(&event, g_input_procs->funcs[DialogEventType_MouseMove]);
}

void update_world_cursor() {
    g_world_cursor = g_zoom.surface_to_world_offset({ g_mouse->x, g_mouse->y });
}

void update_window_focus() {
    if (!g_main_window) {
        return;
    }
    if (display::window_is_focused()) {
        if (g_window_focused) {
            return;
        }
        g_window_focused = true;
        SetCursorPos(g_unfocused_cursor_pos.x, g_unfocused_cursor_pos.y);
    } else {
        g_window_focused = false;
        POINT cursor = {};
        // Windows.h import bypasses cnc-ddraw's fake cursor
        if (GetCursorPos(&cursor)) {
            g_unfocused_cursor_pos = { cursor.x, cursor.y };
        }
    }
}

// 004D1140
u32 is_point_on_console_ui(s32 x, s32 y) {
    const s32 left = g_zoom.source_left();
    const s32 top = g_zoom.source_top();
    const s32 width = g_zoom.visible_width();
    const s32 height = g_zoom.visible_height();
    return x < left || y < top || x >= left + width || y >= top + height;
}

// 004D1940: Button up; releases capture once no other button remains held.
void __cdecl mouse_button_up(u32 button_bit, u32 packed_position, InputProcsFunc* input_proc, u32 event_type) {
    if (g_refocus_swallowed_buttons & button_bit) {
        g_refocus_swallowed_buttons &= ~button_bit;
        return;
    }

    const u16 remaining = u16(~button_bit) & g_mouse_button_state;
    if ((remaining & 0x2a) != 0) {
        return;
    }

    g_mouse_button_state = remaining;
    ReleaseCapture();

    if (button_bit == 0x20) {
        end_middle_drag();
    }

    dispatch_mouse_event(packed_position, input_proc, event_type);
}

// 004D19C0 / 004D1A50: Button down and double-click; takes capture if no other button held.
void __cdecl mouse_button_down(u32 button_bit, u32 packed_position, InputProcsFunc* input_proc, u32 event_type) {
    if (!g_window_focused && display::window_is_focused()) {
        g_window_focused = true;
        g_refocus_swallowed_buttons |= button_bit;
        warp_cursor({ s32(s16(packed_position)), s32(s16(packed_position >> 16)) });
        return;
    }

    const u8 others = u8(~button_bit) & u8(g_mouse_button_state);
    if ((others & 0x2a) != 0) {
        return;
    }

    g_mouse_button_state = u16(g_mouse_button_state | u16(button_bit));
    SetCapture(g_main_window);

    dispatch_mouse_event(packed_position, input_proc, event_type);
}

// 004D24DC: WM_MOUSEMOVE arm; clamps cursor position and sets bit 0.
void window_proc_mouse_move(u32 packed_position) {
    g_mouse_button_state = u16(g_mouse_button_state | 1);

    g_mouse->x = s32(clamp_cursor_x(packed_position));
    g_mouse->y = s32(clamp_cursor_y(packed_position));
}

// 004D12A0: Computes edge scrolling direction and cursor arrow type from g_mouse.
s32 __cdecl edge_scroll_direction(s32* out_x, s32* out_y) {
    const s32 cursor_x = g_mouse->x;
    const s32 cursor_y = g_mouse->y;

    if (g_scroll_wait_for_move != 0) {
        if (g_last_scroll_cursor_x == cursor_x && g_last_scroll_cursor_y == cursor_y) {
            return 0;
        }
        g_scroll_wait_for_move = 0;
    }

    g_last_scroll_cursor_x = cursor_x;
    g_last_scroll_cursor_y = cursor_y;

    const s32 right_edge = s32(g_display.surface_width()) - 2;
    const s32 bottom_edge = s32(g_display.surface_height()) - 2;

    if (cursor_x <= 1 && g_move_to_x > 0) {
        *out_x = -1;
    } else if (cursor_x >= right_edge && g_move_to_x < g_move_to_x_max) {
        *out_x = 1;
    } else {
        *out_x = 0;
    }

    if (cursor_y <= 1 && g_move_to_y > 0) {
        *out_y = -1;
    } else if (cursor_y >= bottom_edge && g_move_to_y < g_move_to_y_max) {
        *out_y = 1;
    } else {
        *out_y = 0;
    }

    if (*out_x == 0 && *out_y == 0) {
        return 0;
    }
    return s32(g_scroll_cursor_types[*out_x + 3 * *out_y + 4]);
}

// 004215E0: Builds the window clip rect into g_cursor_clip_rect. Under cnc-ddraw the
// ClientToScreen here is its fake (a no-op for the game window), so the stored rect
// stays in game client space — correct, because vanilla consumers pass it through the
// game's own ClipCursor import, where cnc-ddraw translates to screen. Never call
// ClipCursor from plugin code with this rect: the plugin's user32 import bypasses
// cnc-ddraw and would apply game coords as screen coords.
void __cdecl set_cursor_clip_rect() {
    POINT top_left = { 0, 0 };
    POINT bottom_right = { s32(g_display.surface_width()), s32(g_display.surface_height()) };

    if (g_main_window) {
        g_client_to_screen(g_main_window, &top_left);
        g_client_to_screen(g_main_window, &bottom_right);
    }

    SetRect(&g_cursor_clip_rect, top_left.x, top_left.y, bottom_right.x, bottom_right.y);
}

void __cdecl refresh_cursor() {
    if (g_middle_drag_cursor) {
        g_set_cursor_iat(nullptr);
        return;
    }

    if (g_current_cursor) {
        HCURSOR cursor = CursorCache::instance().get_cursor(g_current_cursor, u16(g_cursor_frame_index));
        if (cursor) {
            g_set_cursor_iat(cursor);
        }
    }
}

// 004BE060 / 004BE0B0: Toggles software cursor via mouse layer buffers flag.
void __cdecl cursor_refresh() {
    g_screen_layers.mouse.buffers = 1;
    refresh_cursor();
}

void __cdecl refresh_cursor_0() {
    g_screen_layers.mouse.buffers = 0;
}

void begin_middle_drag_cursor(v2s32 grab_point) {
    GrpFile* grp = MiddleDragCursorId < g_cursor_grps.size() ? g_cursor_grps[MiddleDragCursorId] : nullptr;
    if (!grp || grp->frame_count() == 0) {
        grp = g_current_cursor;
    }
    if (grp && grp->frame_count() != 0) {
        const GrpFrame& frame = grp->frame_headers()[0];
        g_current_cursor = grp;
        g_current_cursor_id = MiddleDragCursorId;
        g_cursor_frame_index = 0;
        g_screen_layers.mouse.left = u16(grab_point.x - (CursorHotspotBase - s32(frame.x)));
        g_screen_layers.mouse.top = u16(grab_point.y - (CursorHotspotBase - s32(frame.y)));
        g_screen_layers.mouse.width = frame.w;
        g_screen_layers.mouse.height = frame.h;
        g_screen_layers.mouse.p_update = helper::mouse_layer_draw;
        g_screen_layers.mouse.buffers = 1;
    }

    g_middle_drag_grab = grab_point;
    g_middle_drag_cursor = true;
    g_set_cursor_iat(nullptr);
    g_wants_redraw = 1;
}

void end_middle_drag_cursor() {
    warp_cursor(g_middle_drag_grab);

    // The drag graphic composited into the surface outlives the drag; mark its rect
    // so the next present resamples the world back under it.
    const Layer& cursor = g_screen_layers.mouse;
    mark_rect(s16(cursor.left), s16(cursor.top),
        s16(cursor.left) + s16(cursor.width) - 1, s16(cursor.top) + s16(cursor.height) - 1);

    g_middle_drag_cursor = false;
    g_screen_layers.mouse.buffers = 0;
    g_current_cursor_id = ~0u;
    g_wants_redraw = 1;
}

void __cdecl update_cursor() {
    if (g_middle_drag_cursor) {
        g_set_cursor_iat(nullptr);
        return;
    }

    if (!g_disable_cursor_scroll) {
        s32 out_x = 0;
        s32 out_y = 0;
        s32 cursor_id = edge_scroll_direction(&out_x, &out_y);
        if (cursor_id == 0) {
            cursor_id = s32(hooks::get_cursor_type());
        }
        if (g_current_cursor_id != u32(cursor_id)) {
            g_current_cursor_id = u32(cursor_id);
            GrpFile* new_grp = (cursor_id >= 0 && size_t(cursor_id) < g_cursor_grps.size()) ? g_cursor_grps[cursor_id] : nullptr;
            if (g_current_cursor != new_grp) {
                g_current_cursor = new_grp;
                g_cursor_frame_index = 0;
                if (new_grp) {
                    HCURSOR cursor = CursorCache::instance().get_cursor(new_grp, 0);
                    if (cursor) {
                        g_set_cursor_iat(cursor);
                    }
                }
            }
        }
    }

    if (g_current_cursor && g_current_cursor->frame_count() > 1) {
        const u32 now = GetTickCount();
        if (s32(now - g_cursor_next_tick) >= 0) {
            g_cursor_next_tick = now + CursorFrameMs;
            g_cursor_frame_index++;
            HCURSOR cursor = CursorCache::instance().get_cursor(g_current_cursor, u16(g_cursor_frame_index));
            if (cursor) {
                g_set_cursor_iat(cursor);
            }
        }
    }
}

// 004D1750: WM_ACTIVATEAPP handler to show OS arrow cursor when window is inactive.
void show_os_cursor(u32 inactive) {
    if (inactive) {
        g_set_cursor_iat(LoadCursorA(nullptr, IDC_ARROW));
    } else {
        refresh_cursor();
    }
}

// Vanilla snaps the cursor to the hardcoded 640x480 center at process launch
// (004E087A in run_game) and game start (004D9975 in begin_game); both land in
// the upper-left on widescreen displays.
void center_cursor() {
    const v2s32 center{ s32(g_display.surface_width()) / 2, s32(g_display.surface_height()) / 2 };
    *g_mouse = center;
    warp_cursor(center);
}

} // hooks
