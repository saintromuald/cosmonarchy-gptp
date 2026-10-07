#include "display.h"
#include "zoom.h"
#include "../../config.h"
#include "../../BW/api.h"
#include "../../BW/bwdata.h"

#include <algorithm>

namespace {

u16 cells_across(u16 pixels, u16 cell) {
    return u16((u32(pixels) + cell - 1) / cell);
}

// Smallest power-of-two cell size whose grid fits within 1200 bytes (required by Storm 440).
u16 resolve_cell(u16 width, u16 height) {
    u16 cell = display::MinCellSize;
    while (u32(cells_across(width, cell)) * u32(cells_across(height, cell)) > display::GridCapacity) {
        cell = u16(cell * 2);
    }
    return cell;
}

} // namespace

// Scratch buffer axis: viewport rounded up to terrain tiles, plus one tile margin for sub-tile scrolling.
u32 display::map_axis(u16 view) {
    const u32 tiles = (u32(view) + display::TileSize - 1) / display::TileSize;
    return (tiles + 1) * display::TileSize;
}

void DisplayGeometry::init() {
    m_surface_width = display::SurfaceWidth;
    m_surface_height = display::SurfaceHeight;
    m_cell_size = resolve_cell(m_surface_width, m_surface_height);
    set_min_zoom_percent(zoom::PlayerMinPercent);
}

u16 DisplayGeometry::surface_width() const {
    return m_surface_width;
}

u16 DisplayGeometry::surface_height() const {
    return m_surface_height;
}

// The rendered world extent: display / zoom floor, dword-aligned. Matches the
// zoom crop extent exactly at the floor (source_extent's rounding), so the crop
// never exceeds the buffer.
u16 DisplayGeometry::fov_width() const {
    const s32 extent = s32(u64(display_width()) * 10000 / (u64(m_min_zoom_percent) * 100));
    return u16(std::clamp(align_down(extent, 4), 4, s32(display_width())));
}

u16 DisplayGeometry::fov_height() const {
    const s32 extent = s32(u64(display_height()) * 10000 / (u64(m_min_zoom_percent) * 100));
    return u16(std::clamp(align_down(extent, 4), 4, s32(display_height())));
}

u16 DisplayGeometry::world_width() const {
    return fov_width();
}

u16 DisplayGeometry::world_height() const {
    return fov_height();
}

bool DisplayGeometry::is_widescreen() const {
    return m_surface_width != display::LegacyWidth || m_surface_height != display::LegacyHeight;
}

u16 DisplayGeometry::display_width() const {
    return m_surface_width;
}

u16 DisplayGeometry::display_height() const {
    return surface_height();
}

// The role's zoom floor in percent; sets the rendered world extent (display / floor).
u16 DisplayGeometry::min_zoom_percent() const {
    return u16(m_min_zoom_percent);
}

void DisplayGeometry::set_min_zoom_percent(u32 percent) {
    m_min_zoom_percent = std::clamp(percent, u32(display::MinZoomPercent), u32(zoom::MaxPercent));
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

u8* DisplayGeometry::grid() const {
    return g_refresh_regions;
}

u16 DisplayGeometry::cell_size() const {
    return m_cell_size;
}

u16 DisplayGeometry::grid_cols() const {
    return cells_across(m_surface_width, m_cell_size);
}

u16 DisplayGeometry::grid_rows() const {
    return cells_across(m_surface_height, m_cell_size);
}

u32 DisplayGeometry::grid_bytes() const {
    return u32(grid_cols()) * u32(grid_rows());
}

u32 DisplayGeometry::map_pitch() const {
    return display::map_axis(world_width());
}

u32 DisplayGeometry::map_rows() const {
    return display::map_axis(world_height());
}

u32 DisplayGeometry::map_size() const {
    return map_pitch() * map_rows();
}

u16 DisplayGeometry::map_tile_cols() const {
    return u16(map_pitch() / display::TileSize);
}

u16 DisplayGeometry::map_tile_rows() const {
    return u16(map_rows() / display::TileSize);
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

void apply_display_role() {
    const bool full_fov = bw::in_replay_or_witness();
    g_display.set_min_zoom_percent(full_fov ? zoom::WitnessMinPercent : zoom::PlayerMinPercent);
    g_zoom.begin_game();
}

