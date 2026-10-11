#include "grid.h"
#include "blit.h"
#include "perf_tracker.h"
#include "framebuffer.h"
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

// Dirty-cell lattices live in g_dirty_cell_grids (16px cells)
// Surface marks frive the present (0041E280), world marks the zoom resample
// At 100% the lattices are identical and the world-grid routing below is inert
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
    return g_dirty_cell_grids.surface.get() + y * grid_stride() + x;
}

void mark_cells(u8* out, s32 count) {
    std::fill_n(out, count, u8(1));
}

void fill_grid(u8 value) {
    std::fill_n(g_dirty_cell_grids.surface.get(), g_display.grid_bytes(), value);
}

u32 count_dirty_cells() {
    const u8* cells = g_dirty_cell_grids.surface.get();
    return u32(std::count_if(cells, cells + g_display.grid_bytes(), [](u8 cell) { return cell != 0; }));
}

// Set by the fade branch so flush_region keeps its world-grid fill for the next
// frame's rebuild (fade frames run no walk to consume it first).
bool g_world_grid_preserved = false;

s32 world_grid_cols() {
    return s32(display::cells_across(g_display.world_width(), g_display.cell_size()));
}

s32 world_grid_rows() {
    return s32(display::cells_across(g_display.world_height(), g_display.cell_size()));
}

} // namespace

namespace display {

// World-phase dirty-cell view helpers: the world grid while zoom presents, the
// surface grid otherwise (identical lattices at 100%).
bool world_grid_active() {
    return g_zoom.needs_transform() && g_screen_layers.game.buffers;
}

void fill_world_grid(u8 value) {
    std::fill_n(g_dirty_cell_grids.world.get(), u32(world_grid_cols()) * u32(world_grid_rows()), value);
}

u8* world_phase_cells() {
    return world_grid_active() ? g_dirty_cell_grids.world.get() : g_dirty_cell_grids.surface.get();
}

s32 world_phase_stride() {
    return world_grid_active() ? world_grid_cols() : s32(g_display.grid_cols());
}

s32 world_phase_cols() {
    return world_grid_active() ? world_grid_cols() : s32(g_display.grid_cols());
}

s32 world_phase_rows() {
    return world_grid_active() ? world_grid_rows() : s32(g_display.grid_rows());
}

bool has_pending_draw() {
    if (g_wants_redraw || g_full_redraw || g_repaint_pending || g_dialog_refresh_region) {
        return true;
    }
    for (s32 i = 7; i >= 1; --i) {
        Layer& layer = (&g_screen_layers.mouse)[i];
        if (layer.buffers && (layer.flags & (LayerFlags::NeedsRefresh | LayerFlags::Marked | LayerFlags::CellsDirty)) != LayerFlags::None) {
            return true;
        }
    }
    const u8* cells = g_dirty_cell_grids.surface.get();
    if (std::any_of(cells, cells + g_display.grid_bytes(), [](u8 cell) { return cell != 0; })) {
        return true;
    }
    if (world_grid_active()) {
        const u8* world = g_dirty_cell_grids.world.get();
        const u32 world_count = u32(world_grid_cols()) * u32(world_grid_rows());
        if (std::any_of(world, world + world_count, [](u8 cell) { return cell != 0; })) {
            return true;
        }
    }
    return false;
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
    hooks::patch_image_clip();
    hooks::patch_camera_center_immediates();
    hooks::init_game_layer();
    hooks::update_scroll_limits();
}

} // display

namespace hooks {
// 0041D710: Checks window draw state and marks all cells if repaint is pending.
s32 __cdecl begin_frame() {
    update_window_focus();
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
    display::fill_world_grid(1);
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

// Marks the world-phase grid over the rect (world pixels): the world grid while
// zoom presents, the surface grid otherwise, where world pixels are surface pixels.
void mark_world_rect(s32 left, s32 top, s32 right, s32 bottom) {
    if (!display::world_grid_active()) {
        mark_rect(left, top, right, bottom);
        return;
    }

    const s32 width = s32(g_display.world_width());
    const s32 height = s32(g_display.world_height());
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

    const s32 stride = world_grid_cols();
    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 last_row = cell_of(bottom);
    const s32 cols = cell_of(right) - first_col + 1;

    u8* row = g_dirty_cell_grids.world.get() + first_row * stride + first_col;
    for (s32 y = first_row; y <= last_row; ++y) {
        mark_cells(row, cols);
        row += stride;
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
        std::fill_n(out, cols, color);
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
// Clamps cell indices to the phase lattice to prevent writes beyond the grid;
// raw image rects can carry negative edges (sprites off the surface top/left).
void __cdecl mark_rect_cells(s32* rect) {
    const s32 first_col = std::max(cell_of(rect[0]), 0);
    const s32 first_row = std::max(cell_of(rect[1]), 0);
    const s32 last_col = std::min(cell_ceil(rect[2]), display::world_phase_cols() - 1);
    const s32 last_row = std::min(cell_ceil(rect[3]), display::world_phase_rows() - 1);

    rect[0] = first_col;
    rect[1] = first_row;
    rect[2] = last_col;
    rect[3] = last_row;

    const s32 stride = display::world_phase_stride();
    const s32 cols = last_col - first_col + 1;
    u8* row = display::world_phase_cells() + first_row * stride + first_col;
    while (rect[1] <= last_row) {
        ++rect[1];
        mark_cells(row, cols);
        row += stride;
    }

    // Post-increments rect[1] past the last marked row to match vanilla behavior.
    ++rect[1];
    g_screen_layers.game.flags |= LayerFlags::Marked;
}

// 00497000(eax = object): returns 1 if any cell under object rect (four s16 at +0x20) is dirty.
u32 __cdecl object_is_dirty(const u8* object) {
    const s32 left = *(const s16*)(object + 0x20);
    const s32 top = *(const s16*)(object + 0x22);
    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 cols = cell_of(*(const s16*)(object + 0x28) + left - 1) - first_col + 1;
    const s32 rows = cell_of(*(const s16*)(object + 0x2A) + top - 1) - first_row + 1;

    const s32 stride = display::world_phase_stride();
    const u8* row = display::world_phase_cells() + first_row * stride + first_col;
    for (s32 y = 0; y < rows; ++y) {
        for (s32 x = 0; x < cols; ++x) {
            if (row[x]) {
                return 1;
            }
        }
        row += stride;
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

// 0041E000: Presents marked surface cells to the primary and clears both grids.
void __cdecl flush_region() {
    // The gameplay present runs only while a HUD is loaded; menus present through their own dialog paths.
    if (!g_hud_manager || !g_hud_manager->loaded()) {
        return;
    }

    // Coverage of the surface grid as this present is about to consume it, after
    // every fill and mark the frame made.
    perf::record_dirty_cells(count_dirty_cells(), g_display.grid_bytes());

    const u64 blit_start = perf::now_us();
    present_dirty_cells(g_hud_manager ? g_hud_manager->present_protection() : nullptr);
    fill_grid(0);
    if (!g_world_grid_preserved) {
        display::fill_world_grid(0);
    }
    g_world_grid_preserved = false;
    perf::record_blit(u32(perf::now_us() - blit_start));
}

// rect_is_dirty's world-phase counterpart: any dirty cell under the world-pixel rect.
u8 world_rect_is_dirty(s32 left, s32 top, s32 right, s32 bottom) {
    const s32 first_col = cell_of(left);
    const s32 first_row = cell_of(top);
    const s32 last_row = cell_of(bottom - 1);
    const s32 cols = cell_of(right - 1) - first_col + 1;

    const s32 stride = display::world_phase_stride();
    const u8* row = display::world_phase_cells() + first_row * stride + first_col;
    for (s32 y = first_row; y <= last_row; ++y) {
        for (s32 x = 0; x < cols; ++x) {
            if (row[x]) {
                return 1;
            }
        }
        row += stride;
    }
    return 0;
}

// Maps dirty world cells through the crop/lookup onto surface cells, so the world
// phase's marks drive the resample, present, and HUD invalidation the way the
// single grid does at 100% (where both lattices are the same buffer).
void propagate_world_cells() {
    const auto cell = s32(g_display.cell_size());
    const auto cols = world_grid_cols();
    const auto rows = world_grid_rows();
    const auto world_width = s32(g_display.world_width());
    const auto world_height = s32(g_display.world_height());

    for (s32 row = 0; row < rows; ++row) {
        const u8* row_cells = g_dirty_cell_grids.world.get() + row * cols;
        for (s32 col = 0; col < cols; ++col) {
            if (!row_cells[col]) {
                continue;
            }
            const s32 left = col * cell;
            const s32 top = row * cell;
            const s32 right = std::min(left + cell, world_width);
            const s32 bottom = std::min(top + cell, world_height);

            const zoom::PixelSpan span_x = g_zoom.surface_span_for_world_x(left, right);
            const zoom::PixelSpan span_y = g_zoom.surface_span_for_world_y(top, bottom);
            if (span_x.empty() || span_y.empty()) {
                continue;
            }
            mark_rect(span_x.begin, span_y.begin, span_x.end - 1, span_y.end - 1);
        }
    }
}

// The inverse of the propagation: surface marks made outside the world phase
// (dialog updates, text expiry, cursor rects) still need world content restored
// beneath them. Marks the world cells each dirty surface cell samples so the
// world phase re-blits them from the ring buffer before the resample reads the
// crop - the crop region on the surface holds the previous frame's magnified
// output otherwise, not world pixels.
void back_propagate_surface_cells() {
    const s32 cell = s32(g_display.cell_size());
    const s32 cols = s32(g_display.grid_cols());
    const s32 rows = s32(g_display.grid_rows());
    const u8* cells = g_dirty_cell_grids.surface.get();

    for (s32 row = 0; row < rows; ++row) {
        for (s32 col = 0; col < cols; ++col) {
            if (!cells[row * cols + col]) {
                continue;
            }
            const s32 out_left = col * cell;
            const s32 out_top = row * cell;
            const s32 out_right = std::min(out_left + cell, s32(g_display.surface_width()));
            const s32 out_bottom = std::min(out_top + cell, s32(g_display.surface_height()));

            const zoom::PixelSpan span_x = g_zoom.world_span_for_surface_x(out_left, out_right);
            const zoom::PixelSpan span_y = g_zoom.world_span_for_surface_y(out_top, out_bottom);
            if (span_x.empty() || span_y.empty()) {
                continue;
            }
            mark_world_rect(span_x.begin, span_y.begin, span_x.end - 1, span_y.end - 1);
        }
    }
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

    if (g_full_redraw) {
        *g_current_surface = (Bitmap*)g_game_screen_buffer;
        auto data = g_game_screen_buffer->data;
        std::fill_n(data, size_t(g_game_screen_buffer->width) * g_game_screen_buffer->height, 0);
        s32 rect[4] = { 0, 0, width, height };
        helper::present_rect(rect);
        // The full-screen present copied every pixel already; clearing the grid keeps the
        // flush below from re-copying them through the storm mask.
        fill_grid(0);
        // The black wipe invalidated the world content; full-fill so the next frame rebuilds it.
        display::fill_world_grid(1);
        g_world_grid_preserved = true;
    } else {
        hooks::update_world_cursor();

        const bool zoom_frame = g_zoom.needs_transform() && g_screen_layers.game.buffers;

        // While magnified, the world layers composite into the dedicated world buffer
        // and the i==2 resample copies the crop onto the screen surface; at 100% the
        // world layers draw straight onto the screen as before.
        *g_current_surface = zoom_frame ? (Bitmap*)&g_world_surface : (Bitmap*)g_game_screen_buffer;

        bool zoom_full = false;
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

            // Full-world frames fill both grids: a running transition or anchor,
            // camera motion, a zoom percent change, or a vanilla full-world request
            // (NeedsRefresh on the game layer - set on scroll, camera jumps, vision flips)
            // moves content everywhere, so the whole crop rescales. At rest the
            // tick's world marks drive the frame instead: they propagate onto
            // surface cells and only those are rescaled and presented.
            static v2s32 presented_camera = v2s32{ -1, -1 };
            static u32 presented_percent = 0;
            const v2s32 current_camera{ g_move_to_x, g_move_to_y };
            zoom_full = g_zoom.transition_running()
                || g_zoom.anchor_active()
                || (g_screen_layers.game.flags & LayerFlags::NeedsRefresh) != LayerFlags::None
                || current_camera != presented_camera
                || g_zoom.percent() != presented_percent;
            presented_camera = current_camera;
            presented_percent = g_zoom.percent();

            if (zoom_full || g_frame_overlay.registered()) {
                zoom_full = true;
                fill_grid(1);
                display::fill_world_grid(1);
            } else {
                back_propagate_surface_cells();

                // High world-mark coverage makes a full-frame reprocess cheaper than many
                // rect passes - and the full resample may only run on fully reprocessed
                // content: presenting unmarked cells would surface stale sprite-draw
                // content (bright flashes in fog) as flicker whenever coverage crosses
                // the threshold. Promote to a full frame *before* the walk so everything
                // gets blitted+fogged first.
                const u32 world_cells = u32(world_grid_cols()) * u32(world_grid_rows());
                u32 world_marks = 0;
                for (u32 i = 0; i < world_cells; ++i) {
                    world_marks += g_dirty_cell_grids.world[i] != 0;
                }
                if (world_marks * 5 >= world_cells * 3) {
                    zoom_full = true;
                    fill_grid(1);
                    display::fill_world_grid(1);
                }
            }
        }

        // Dirty the full grid while an overlay is registered to clear previous overlay pixels.
        if (!zoom_frame && g_frame_overlay.registered()) {
            fill_grid(1);
        }

        // 0041CA00 frees dialog refresh regions at frame end; invalidate dirty dialogs before
        // layer updates. Unzoomed frames keep the original pre-walk position so 100% behavior
        // is unchanged; zoomed frames invalidate from the i==2 slot after the propagation.
        if (!zoom_frame && g_hud_manager) {
            g_hud_manager->invalidate_dirty_dialogs();
        }

        // Walk the eight slots in draw order (bottom-up); the spares at 7/6 are
        // never populated, and the mouse layer (index 0) is drawn separately below.
        for (s32 i = 7; i >= 1; --i) {
            Layer& layer = (&g_screen_layers.mouse)[i];

            // Between the world layers (i > 2) and the UI layers: map this frame's world
            // marks - tick-time marks plus the walk's own fog and sprite-erase marks -
            // onto surface cells, then invalidate dialogs over changed world, then
            // resample the marked output before the HUD draws on top. 0041CA00 frees
            // dialog refresh regions at frame end, so this stays ahead of the dialog
            // layer updates at i <= 2 below.
            if (i == 2) {
                // Back to the screen surface for the resample and everything drawn on top.
                *g_current_surface = (Bitmap*)g_game_screen_buffer;

                if (zoom_frame && !zoom_full) {
                    propagate_world_cells();
                }
                if (zoom_frame && g_hud_manager) {
                    g_hud_manager->invalidate_dirty_dialogs();
                }
                if (zoom_frame) {
                    const u64 resample_start = perf::now_us();
                    if (zoom_full) {
                        g_zoom.scale_world(g_game_screen_buffer->data, g_world_surface.data, s32(g_world_surface.width));
                    } else {
                        const u32 dirty = count_dirty_cells();
                        if (dirty) {
                            g_zoom.scale_world_region(g_game_screen_buffer->data, g_world_surface.data, s32(g_world_surface.width));
                        }
                    }
                    perf::record_resample(u32(perf::now_us() - resample_start));

                    // Draw unscaled screen-space text and shapes (message log, objectives, custom shapes).
                    if (g_zoom.magnified()) {
                        hooks::draw_deferred_screen_text();
                        hooks::draw_deferred_shapes();
                    }
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

            if ((layer.flags & (LayerFlags::NeedsRefresh | LayerFlags::AlwaysUpdate)) == LayerFlags::None) {
                // World layers (3-7) check the world-phase view; UI layers the surface grid.
                const u8 dirty = (i >= 3)
                    ? world_rect_is_dirty(layer.left, layer.top, layer.left + layer.width, layer.top + layer.height)
                    : rect_is_dirty(layer.left, layer.top, layer.left + layer.width, layer.top + layer.height);
                if (dirty) {
                    layer.flags |= LayerFlags::CellsDirty;
                } else if ((layer.flags & LayerFlags::Marked) == LayerFlags::None) {
                    continue;
                }
            }

            helper::call_layer_update((void*)layer.p_update, layer.p_surface, &clip);
            layer.flags &= ~(LayerFlags::NeedsRefresh | LayerFlags::Marked | LayerFlags::CellsDirty);
        }

        if (hooks::g_middle_drag_cursor && g_screen_layers.mouse.buffers && g_screen_layers.mouse.p_update) {
            Layer& cursor = g_screen_layers.mouse;
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

void __cdecl refresh_world_cells() {
    const s32 cell = s32(g_display.cell_size());
    const s32 view_width = s32(g_display.world_width());
    const s32 view_height = s32(g_display.world_height());
    const s32 cols = (view_width + cell - 1) / cell;
    const s32 scratch_pitch = s32(g_display.map_pitch());
    const s32 scratch_size = s32(g_display.map_size());

    // Viewport top-left byte offset in world scratch ring buffer.
    s32 offset = s32((u32(g_move_to_y) * u32(scratch_pitch) + u32(g_move_to_x)) % u32(scratch_size));

    const s32 stride = display::world_phase_stride();
    const u8* grid_base = display::world_phase_cells();
    for (s32 y = 0; y < view_height; y += cell) {
        auto row = grid_base + (y / cell) * stride;
        s32 col = 0;
        while (col < cols) {
            if (offset >= scratch_size) {
                offset -= scratch_size;
            }
            if (row[col] == 1) {
                // Exact dirty-run coverage: the resample can sample any world-buffer pixel
                // (output cells straddle world cells at non-integer magnifications), so
                // unlike vanilla's screen blit this must not write the one extra cell
                // past the run - that cell would sit unfogged with nothing to re-fog it.
                s32 run = 1;
                while (col + run < cols && row[col + run]) {
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

    Layer& game = g_screen_layers.game;
    game.flags = LayerFlags::None;
    game.left = 0;
    game.top = 0;
    game.width = u16(view_width);
    game.height = u16(view_height);
    game.p_surface = nullptr;

    // Read from patch site at 0x004BD68D (hooked by draw_hook.cpp), not hardcoded 0x004BD580.
    game.p_update = g_draw_game_proc;

    fill_grid(1);
    display::fill_world_grid(1);

    // 0041E1E0: inlined dirty marking for dialog and building placement layers.
    for (s32 i = 3; i <= 5; ++i) {
        Layer& layer = (&g_screen_layers.mouse)[i];
        layer.flags |= LayerFlags::NeedsRefresh;
        mark_rect(s16(layer.left), s16(layer.top), s16(layer.left) + s16(layer.width) - 1, s16(layer.top) + s16(layer.height) - 1);
    }
}

} // hooks
