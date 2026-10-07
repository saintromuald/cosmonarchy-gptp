#include "grid.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../systems/display/frame_overlay.h"
#include "../../systems/display/zoom.h"
#include "../interface/hud/hud_manager.h"
#include "screen_text.h"
#include "mouse.h"
#include "sprites.h"
#include "camera.h"
#include "../../graphics/draw_hook.h"
#include "../../graphics/Bitmap.h"
#include "../../storm/storm.h"

#include <algorithm>

// 1200-byte dirty cell grid at 0x006CEFF8. 0041E280 builds Storm update regions from it.
// Cell size varies dynamically via g_display.cell_size() so the lattice fits the fixed buffer.
namespace {

s32 cell_of(s32 pixel) {
    return pixel / s32(g_display.cell_size());
}

// Rounded up, for an edge that has to be covered rather than merely started.
s32 cell_ceil(s32 pixel) {
    const s32 cell = s32(g_display.cell_size());
    return (pixel + cell - 1) / cell;
}

s32 grid_stride() {
    return s32(g_display.grid_cols());
}

u8* cell_at(s32 x, s32 y) {
    return g_display.grid() + y * grid_stride() + x;
}

void mark_cells(u8* out, s32 count) {
    for (s32 i = 0; i < count; ++i) {
        out[i] = 1;
    }
}

void fill_grid(u8 value) {
    auto cells = g_display.grid();
    const u32 count = g_display.grid_bytes();
    for (u32 i = 0; i < count; ++i) {
        cells[i] = value;
    }
}

Layers& g_layers = *g_screen_layers;

// Layer flags byte at offset +1; tests bits beyond bit 0.
u8& layer_flags(Layer& layer) {
    return *((u8*)&layer + 1);
}

} // namespace

namespace hooks {
// 0041D710: Checks window draw state and marks all cells if repaint is pending.
s32 __cdecl begin_frame() {
    if (!g_ddraw_ready || !g_window_active || !g_main_window) {
        return 0;
    }
    if (IsIconic(g_main_window)) {
        return 0;
    }
    if (!g_repaint_pending) {
        return 1;
    }

    SDrawRealizePalette();
    fill_grid(1);
    helper::redraw_main_dialog();
    g_repaint_pending = 0;
    return 1;
}

// 0041DE20: Returns 1 if any cell in the rectangle (exclusive right/bottom) is dirty.
u8 __stdcall rect_is_dirty(s32 left, s32 top, s32 right, s32 bottom) {
    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 last_row = cell_of(bottom - 1);
    const s32 cols = cell_of(right - 1) - first_col + 1;

    auto row = (const u8*)cell_at(first_col, first_row);
    for (s32 y = first_row; y <= last_row; ++y) {
        for (s32 x = 0; x < cols; ++x) {
            if (row[x]) {
                return 1;
            }
        }
        row += grid_stride();
    }
    return 0;
}

// 0041E0D0: Marks all cells touching screen rectangle (inclusive bounds clamped to surface).
void __cdecl mark_rect(s32 left, s32 top, s32 right, s32 bottom) {
    const s32 width = s32(g_display.surface_width());
    const s32 height = s32(g_display.surface_height());

    if (left < 0) {
        left = 0;
    } else if (left >= width) {
        return;
    }
    if (right < 0) {
        return;
    }
    if (right > width - 1) {
        right = width - 1;
    }
    if (top < 0) {
        top = 0;
    } else if (top >= height) {
        return;
    }
    if (bottom < 0) {
        return;
    }
    if (bottom > height - 1) {
        bottom = height - 1;
    }

    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 last_row = cell_of(bottom);
    const s32 cols = cell_of(right) - first_col + 1;

    auto row = cell_at(first_col, first_row);
    for (s32 y = first_row; y <= last_row; ++y) {
        mark_cells(row, cols);
        row += grid_stride();
    }
}

// 004E1D20: Fills surface rectangle with draw color; guards against null surface buffer.
void __stdcall draw_box(s32 x, s32 y, s32 box_w, s32 box_h) {
    const s32 rows = s32(u16(box_h));
    if (rows <= 0) {
        return;
    }

    Bitmap* dest = *g_current_surface;
    if (!dest || !dest->buffer) {
        return;
    }

    const s32 pitch = s32(s16(dest->width));
    const s32 cols = s32(u16(box_w));
    const u8 color = u8(*g_draw_color_index);

    for (s32 row = 0; row < rows; ++row) {
        u8* out = dest->buffer + s32(s16(y + row)) * pitch + s32(s16(x));
        for (s32 i = 0; i < cols; ++i) {
            out[i] = color;
        }
    }
}

// 0045CC90(eax = rect), called by 0047D920: clips half-open screen rect to world viewport.
// Returns non-zero when nothing remains.
u32 __cdecl clip_rect_to_world(s32* rect) {
    const s32 width = s32(g_display.world_width());
    const s32 height = s32(g_display.world_height());

    if (rect[2] <= 0 || rect[3] <= 0 || rect[0] >= width || rect[1] >= height) {
        return 1;
    }

    rect[0] = std::max(rect[0], 0);
    rect[1] = std::max(rect[1], 0);
    rect[2] = std::min(rect[2], width);
    rect[3] = std::min(rect[3], height);
    return 0;
}

// 0042D280(esi = rect): marks cells in rect, rewriting rect into cell coordinates.
// Clamps cell_ceil to grid bounds to prevent writes beyond the 1200-byte grid.
void __cdecl mark_rect_cells(s32* rect) {
    const s32 first_col = cell_of(rect[0]);
    const s32 first_row = cell_of(rect[1]);
    const s32 last_col = std::min(cell_ceil(rect[2]), s32(g_display.grid_cols()) - 1);
    const s32 last_row = std::min(cell_ceil(rect[3]), s32(g_display.grid_rows()) - 1);

    rect[0] = first_col;
    rect[1] = first_row;
    rect[2] = last_col;
    rect[3] = last_row;

    const s32 cols = last_col - first_col + 1;
    auto row = cell_at(first_col, first_row);
    while (rect[1] <= last_row) {
        ++rect[1];
        mark_cells(row, cols);
        row += grid_stride();
    }

    // Post-increments rect[1] past the last marked row to match vanilla behavior.
    ++rect[1];
    layer_flags(g_layers.game) |= 2;
}

// 00497000(eax = object): returns 1 if any cell under object rect (four s16 at +0x20) is dirty.
u32 __cdecl object_is_dirty(const u8* object) {
    const s32 left = *(const s16*)(object + 0x20);
    const s32 top = *(const s16*)(object + 0x22);
    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 cols = cell_of(*(const s16*)(object + 0x28) + left - 1) - first_col + 1;
    const s32 rows = cell_of(*(const s16*)(object + 0x2A) + top - 1) - first_row + 1;

    auto row = (const u8*)cell_at(first_col, first_row);
    for (s32 y = 0; y < rows; ++y) {
        for (s32 x = 0; x < cols; ++x) {
            if (row[x]) {
                return 1;
            }
        }
        row += grid_stride();
    }
    return 0;
}

// 0048CB80(eax = index): marks the refresh bounds for one text slot. The bounds come from
// the shared screen_text layout constants (screen_text.h) so marking follows any relocation.
void __cdecl expire_text_line(u32 index) {
    const s32 bottom = s32(g_display.surface_height()) - 1;

    if (index == CenterTextSlot) {
        s32 y = s32(g_text_area_y) + CenterTextTop;
        if (y > bottom) {
            y = bottom;
        }
        const s32 first_row = cell_of(CenterTextTop);
        const s32 last_row = cell_of(y);
        if (last_row < first_row) {
            return;
        }
        mark_cells(cell_at(0, first_row), (last_row - first_row + 1) * grid_stride());
        return;
    }

    if (index >= TopRightTextSlot) {
        s32 y = s32(g_text_area_y) + TopRightTextTop;
        if (y > bottom) {
            y = bottom;
        }
        const s32 first_col = cell_of(TopRightTextLeft);
        const s32 cols = cell_of(TopRightTextRight) - first_col + 1;
        const s32 first_row = cell_of(TopRightTextTop);
        const s32 last_row = cell_of(y);
        if (last_row < first_row) {
            return;
        }
        auto row = cell_at(first_col, first_row);
        for (s32 i = first_row; i <= last_row; ++i) {
            mark_cells(row, cols);
            row += grid_stride();
        }
        return;
    }

    const s32 slot = (s32(index) - s32(g_text_line_first) + MessageLogSlotCount) % MessageLogSlotCount;
    const s32 offset = s32(g_text_line_height) * slot;
    mark_rect(MessageLogLeft, offset + MessageLogTop, MessageLogRight, s32(g_text_area_y) + offset + MessageLogTop);
}

// 0041E000: Builds Storm update region from marked cells, presents screen, and clears grid.
void __cdecl flush_region() {
    if (!g_storm_blit_chain || !g_storm_update_region) {
        return;
    }
    STransDelete(g_storm_update_region);
    STransIntersectDirtyArray(g_storm_blit_chain, g_display.grid(), 3, &g_storm_update_region);
    helper::present_screen();
    fill_grid(0);
}

// 0041E280: Composites dirty layers to screen surface, builds Storm update region, and presents.
void __cdecl present_frame() {
    if (!g_game_screen_buffer->data) {
        return;
    }

    // Skips presentation if frame overlay asks to drop frame; preserves dirty cells.
    if (g_frame_overlay.skip()) {
        return;
    }

    const s32 width = s32(g_display.surface_width());
    const s32 height = s32(g_display.surface_height());

    *g_current_surface = (Bitmap*)g_game_screen_buffer;

    if (g_full_redraw) {
        auto data = g_game_screen_buffer->data;
        std::fill_n(data, size_t(g_game_screen_buffer->width) * g_game_screen_buffer->height, 0);
        s32 rect[4] = { 0, 0, width, height };
        helper::present_rect(rect);
    } else {
        hooks::update_world_cursor();

        const bool zoom_frame = g_zoom.needs_transform() && g_layers.game.buffers;
        if (zoom_frame) {
            g_zoom.update_transform();

            // Applies remaining zoom camera travel during transition (once per drawn frame).
            v2s32 camera;
            if (g_zoom.resolve_camera_anchor(&camera)) {
                helper::move_screen({ u16(camera.x), u16(camera.y) });
                g_zoom.update_transform(false);
            }

            // The console overscroll allowance scales with the current zoom, so the
            // scroll clamps move while a zoom transition runs. Re-derive them when
            // the percent changes so edge scroll and vanilla camera moves stop at
            // the new max; the camera itself was re-clamped by the anchor above.
            static u32 clamped_zoom_percent = 0;
            if (g_zoom.percent() != clamped_zoom_percent) {
                clamped_zoom_percent = g_zoom.percent();
                update_scroll_limits();
            }

            // Dirties full grid so entire zoomed world rectangle is rescaled and presented.
            fill_grid(1);
        }

        // Dirty the full grid while an overlay is registered to clear previous overlay pixels.
        if (!zoom_frame && g_frame_overlay.registered()) {
            fill_grid(1);
        }

        // 0041CA00 frees dialog refresh regions at frame end; invalidate dirty dialogs before layer updates.
        if (g_hud_manager) {
            g_hud_manager->invalidate_dirty_dialogs();
        }

        // Walk the eight slots in draw order (bottom-up); the spares at 7/6 are
        // never populated, and the mouse layer (index 0) is drawn separately below.
        for (s32 i = 7; i >= 1; --i) {
            Layer& layer = (&g_layers.mouse)[i];

            // Scale world layer between world layers (i > 2) and UI layers (HUD/tooltips).
            if (zoom_frame && i == 2) {
                g_zoom.scale_world(g_game_screen_buffer->data);

                // Draw unscaled screen-space text and shapes (message log, objectives, custom shapes).
                if (g_zoom.magnified()) {
                    hooks::draw_deferred_screen_text();
                    hooks::draw_deferred_shapes();
                }
            }

            if (!layer.buffers) {
                continue;
            }

            Bounds clip;
            clip.left = s16(-s32(layer.left));
            clip.top = s16(-s32(layer.top));
            clip.right = s16(clip.left + width - 1);
            clip.bottom = s16(clip.top + height - 1);
            clip.width = s16(width);
            clip.height = s16(height);

            u8& flags = layer_flags(layer);
            if (!(flags & 0x21)) {
                if (rect_is_dirty(layer.left, layer.top, layer.left + layer.width, layer.top + layer.height)) {
                    flags |= 4;
                } else if (!(flags & 2)) {
                    continue;
                }
            }

            helper::call_layer_update((void*)layer.p_update, layer.p_surface, &clip);
            flags &= 0xF8;
        }

        if (hooks::g_middle_drag_cursor && g_layers.mouse.buffers && g_layers.mouse.p_update) {
            Layer& cursor = g_layers.mouse;
            Bounds clip;
            clip.left = s16(-s32(cursor.left));
            clip.top = s16(-s32(cursor.top));
            clip.right = s16(clip.left + width - 1);
            clip.bottom = s16(clip.top + height - 1);
            clip.width = s16(width);
            clip.height = s16(height);
            helper::call_layer_update((void*)cursor.p_update, cursor.p_surface, &clip);
            mark_rect(s16(cursor.left), s16(cursor.top), s16(cursor.left) + s16(cursor.width) - 1, s16(cursor.top) + s16(cursor.height) - 1);
        }
    }

    flush_region();

    *g_current_surface = nullptr;
}

bool has_pending_draw() {
    if (g_wants_redraw || g_full_redraw || g_repaint_pending || g_dialog_refresh_region) {
        return true;
    }
    for (s32 i = 7; i >= 1; --i) {
        Layer& layer = (&g_layers.mouse)[i];
        if (layer.buffers && (layer_flags(layer) & 7)) {
            return true;
        }
    }
    const u8* cells = g_display.grid();
    return std::any_of(cells, cells + g_display.grid_bytes(), [](u8 cell) { return cell != 0; });
}

void __cdecl refresh_world_cells() {
    const s32 cell = s32(g_display.cell_size());
    const s32 view_width = s32(g_display.world_width());
    const s32 view_height = s32(g_display.world_height());
    const s32 cols = (view_width + cell - 1) / cell;
    const s32 scratch_pitch = s32(g_display.map_pitch());
    const s32 scratch_size = s32(g_display.map_size());

    // Viewport top-left byte offset in world scratch ring buffer.
    s32 offset = s32((u32(g_move_to_y) * u32(scratch_pitch) + u32(g_move_to_x)) % u32(scratch_size));

    for (s32 y = 0; y < view_height; y += cell) {
        auto row = (const u8*)cell_at(0, y / cell);
        s32 col = 0;
        while (col < cols) {
            if (offset >= scratch_size) {
                offset -= scratch_size;
            }
            if (row[col] == 1) {
                // Span includes one extra cell to match vanilla span/offset stepping.
                s32 run = 1;
                s32 scan = col;
                while (col + run < cols && row[scan]) {
                    ++scan;
                    ++run;
                }

                helper::blit_map_span(y, col * cell, run * cell, offset);

                offset += run * cell - cell;
                if (offset >= scratch_size) {
                    offset -= scratch_size;
                }
                col += run - 1;
            }
            ++col;
            offset += cell;
        }

        // Advance to left edge of next cell row.
        offset += cell * scratch_pitch - view_width;
        if (offset >= scratch_size) {
            offset -= scratch_size;
        }
    }
}

// 004BD630: Initializes game layer dimensions, cursor limits, and marks full grid.
void __cdecl init_game_layer() {
    const s32 view_width = s32(g_display.world_width());
    const s32 view_height = s32(g_display.world_height());

    // The cursor limit gates screen-space consumers (vanilla's right-click
    // trampoline 00484620, get_cursor_type) that test the raw cursor against the
    // gameplay area. Under zoom the world fills the whole display rect, so the
    // limit is the display — not the world extent (00484620's PtInRect with a
    // 960x540 rect rejected every click beyond the top-left quarter at 2x).
    SetRect(g_cursor_limit_rect, 0, 0, s32(g_display.display_width()) - 1, s32(g_display.display_height()) - 1);

    Layer& game = g_layers.game;
    layer_flags(game) = 0;
    game.left = 0;
    game.top = 0;
    game.width = u16(view_width);
    game.height = u16(view_height);
    game.p_surface = nullptr;

    // Read from patch site at 0x004BD68D (hooked by draw_hook.cpp), not hardcoded 0x004BD580.
    game.p_update = g_draw_game_proc;

    fill_grid(1);

    // 0041E1E0: inlined dirty marking for dialog and building placement layers.
    for (s32 i = 3; i <= 5; ++i) {
        Layer& layer = (&g_layers.mouse)[i];
        layer_flags(layer) |= 1;
        mark_rect(s16(layer.left), s16(layer.top), s16(layer.left) + s16(layer.width) - 1, s16(layer.top) + s16(layer.height) - 1);
    }
}

// Re-resolves the replay/witness role and reapplies everything derived from it:
// the zoom floor (and with it the world extent), the GRP clip immediates, the
// camera centering immediates, the game layer geometry, and the scroll clamps.
// Safe at any frame — every extent-sized buffer allocates for the full FOV, so
// role changes only move logical bounds. Called at game start and from the
// witness_mode trigger action, which can flip the local player's role at any
// time (CM maps assign witnesses via a first-frame trigger).
void refresh_display_role() {
    apply_display_role();
    patch_image_clip();
    patch_camera_center_immediates();
    init_game_layer();
    update_scroll_limits();
}

} // hooks
