#include "display.h"
#include "zoom.h"
#include "../../config.h"
#include "../../BW/api.h"
#include "../../BW/bwdata.h"

#include <algorithm>

namespace {

// Smallest power-of-two cell size whose grid fits within GridCapacity bytes.
u16 resolve_cell(u16 width, u16 height) {
    u16 cell = display::MinCellSize;
    while (u32(display::cells_across(width, cell)) * u32(display::cells_across(height, cell)) > display::GridCapacity) {
        cell = u16(cell * 2);
    }
    return cell;
}

} // namespace

bool display::window_is_focused() {
    return g_main_window && GetForegroundWindow() == g_main_window;
}

void DisplayGeometry::init() {
    m_surface_width = display::SurfaceWidth;
    m_surface_height = display::SurfaceHeight;
    m_cell_size = resolve_cell(m_surface_width, m_surface_height);
    m_grid_cols = display::cells_across(m_surface_width, m_cell_size);
    m_grid_rows = display::cells_across(m_surface_height, m_cell_size);
    set_min_zoom_percent(zoom::PlayerMinPercent);
}

// The rendered world extent: display / zoom floor, dword-aligned. Matches the
// zoom crop extent exactly at the floor (source_extent's rounding), so the crop
// never exceeds the buffer.
void DisplayGeometry::recompute_world_extents() {
    const s32 world_width = s32(u64(m_surface_width) * 10000 / (u64(m_min_zoom_percent) * 100));
    const s32 world_height = s32(u64(m_surface_height) * 10000 / (u64(m_min_zoom_percent) * 100));
    m_world_width = u16(std::clamp(align_down(world_width, 4), 4, s32(m_surface_width)));
    m_world_height = u16(std::clamp(align_down(world_height, 4), 4, s32(m_surface_height)));
    m_map_pitch = display::map_axis(m_world_width);
    m_map_rows = display::map_axis(m_world_height);
    m_map_size = m_map_pitch * m_map_rows;
    m_map_tile_cols = u16(m_map_pitch / display::TileSize);
    m_map_tile_rows = u16(m_map_rows / display::TileSize);
}

bool DisplayGeometry::is_widescreen() const {
    return m_surface_width != display::LegacyWidth || m_surface_height != display::LegacyHeight;
}

// The role's zoom floor in percent; sets the rendered world extent (display / floor).
void DisplayGeometry::set_min_zoom_percent(u32 percent) {
    m_min_zoom_percent = std::clamp(percent, u32(display::MinZoomPercent), u32(zoom::MaxPercent));
    recompute_world_extents();
}

u16 DisplayGeometry::center_offset_x() const {
    return u16(world_width() / 2);
}

u16 DisplayGeometry::center_offset_y() const {
    return u16(world_height() / 2);
}

u16 DisplayGeometry::center_offset_tile_x() const {
    return u16(world_width() / (2 * display::TileSize));
}

u16 DisplayGeometry::center_offset_tile_y() const {
    return u16(world_height() / (2 * display::TileSize));
}

s32 DisplayGeometry::scroll_max_x(s32 map_width_pixels) const {
    const s32 limit = map_width_pixels - s32(world_width());
    return limit > 0 ? limit : 0;
}

s32 DisplayGeometry::scroll_max_y(s32 map_height_pixels) const {
    // Console band allowance, world-scaled by the current zoom: the strip must
    // cover only the band on screen, so the scroll range past the map bottom
    // shrinks as the camera zooms in. 8-aligned so scroll_down's exact clamp
    // preserves 8-aligned camera positions.
    const u32 magnification = g_zoom.percent();
    const s32 margin = align_up(s32(display::BottomConsoleBandPx) * 100 / s32(magnification), 8);
    const s32 limit = map_height_pixels - s32(world_height()) + margin;
    return limit > 0 ? limit : 0;
}

u16 DisplayGeometry::cursor_max_x() const {
    return u16(surface_width() - 1);
}

u16 DisplayGeometry::cursor_max_y() const {
    return u16(surface_height() - 1);
}

u16 DisplayGeometry::mask_stride() const {
    return u16(map_tile_cols() + 3);
}

u16 DisplayGeometry::mask_rows() const {
    return u16(map_tile_rows() + 3);
}

u32 DisplayGeometry::mask_bytes() const {
    return u32(mask_stride()) * u32(mask_rows());
}

u16 DisplayGeometry::vision_stride() const {
    return u16((map_tile_cols() + 1) * 4);
}

u16 DisplayGeometry::vision_rows() const {
    return u16((map_tile_rows() + 1) * 4);
}

u32 DisplayGeometry::vision_bytes() const {
    return u32(vision_stride()) * u32(vision_rows());
}

u32 DisplayGeometry::full_mask_bytes() const {
    const u32 cols = display::map_axis(display_width()) / display::TileSize;
    const u32 rows = display::map_axis(display_height()) / display::TileSize;
    return (cols + 3) * (rows + 3);
}

u32 DisplayGeometry::full_vision_bytes() const {
    const u32 cols = display::map_axis(display_width()) / display::TileSize;
    const u32 rows = display::map_axis(display_height()) / display::TileSize;
    return (cols + 1) * 4 * (rows + 1) * 4;
}

void DirtyCellGrids::init() {
    surface = std::make_unique<u8[]>(g_display.grid_bytes());
    world = std::make_unique<u8[]>(g_display.grid_bytes());
}

void apply_display_role() {
    const bool full_fov = bw::in_replay_or_witness();
    g_display.set_min_zoom_percent(full_fov ? zoom::WitnessMinPercent : zoom::PlayerMinPercent);
    g_zoom.begin_game();
}

