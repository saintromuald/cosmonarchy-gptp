#pragma once
#include "../../types.h"

#include <memory>

namespace display {

constexpr u16 LegacyWidth = 640;
constexpr u16 LegacyWorldHeight = 400;
constexpr u16 LegacyHeight = 480;

// Fixed presentation size: UI art is authored for this resolution and cannot scale.
constexpr u16 SurfaceWidth = 1920;
constexpr u16 SurfaceHeight = 1080;
static_assert(SurfaceWidth % 4 == 0 && SurfaceHeight % 4 == 0); // dword-aligned blit pitch

// Absolute zoom floor: magnifying below 100% would show more world than the window.
constexpr u16 MinZoomPercent = 100;

constexpr u16 TileSize = 32;

// Surface-pixel height of the bottom console band. The camera may scroll past
// the map bottom far enough to cover one band's worth of display, world-scaled
// by the current zoom so the overscroll strip stays band-sized at every
// magnification.
constexpr u16 BottomConsoleBandPx = 200;

constexpr u16 MinCellSize = 16;

constexpr u16 MaxWidth = 1920;
constexpr u16 MaxHeight = 1200;

constexpr u32 LegacyMapSize = 0x49800;

// Cells needed to cover pixels, rounded up
constexpr u16 cells_across(u16 pixels, u16 cell) {
    return u16((u32(pixels) + cell - 1) / cell);
}

// Dirty-cell budget resized so the fixed 1920x1080 surface keeps 16px cells
constexpr u32 GridCapacity = u32(cells_across(SurfaceWidth, MinCellSize))
    * u32(cells_across(SurfaceHeight, MinCellSize));

// Scratch buffer axis: viewport rounded up to terrain tiles, plus one tile margin for sub-tile scrolling.
constexpr u32 map_axis(u16 view) {
    const u32 tiles = (u32(view) + TileSize - 1) / TileSize;
    return (tiles + 1) * TileSize;
}

bool window_is_focused();

} // display

class DisplayGeometry {
public:
    void init();

    u16 surface_width() const { return m_surface_width; }
    u16 surface_height() const { return m_surface_height; }

    u16 fov_width() const { return m_world_width; }
    u16 fov_height() const { return m_world_height; }

    // Rendered world area shared by fog (004808E0), dirty-cell walker (004BCDC0), and screen copy (0040C2BD).
    u16 world_width() const { return m_world_width; }
    u16 world_height() const { return m_world_height; }

    bool is_widescreen() const;

    // The zoom output rect: where the magnified world is drawn on the surface
    // (the full surface rect).
    u16 display_width() const { return m_surface_width; }
    u16 display_height() const { return m_surface_height; }

    // The role's zoom floor in percent; sets the rendered world extent
    // (display / floor). Applied per game via apply_display_role() once the
    // replay/witness role is known.
    u16 min_zoom_percent() const { return u16(m_min_zoom_percent); }
    void set_min_zoom_percent(u32 percent);

    // Half-viewport centering offsets in pixels and whole tiles (replaces vanilla literals 320/200/10/6).
    u16 center_offset_x() const;
    u16 center_offset_y() const;
    u16 center_offset_tile_x() const;
    u16 center_offset_tile_y() const;

    // Max camera scroll position. The y axis adds the zoom-scaled console
    // overscroll margin so the lowest map rows can clear the bottom HUD band
    // without the strip over-covering the display at higher magnification.
    s32 scroll_max_x(s32 map_width_pixels) const;
    s32 scroll_max_y(s32 map_height_pixels) const;

    u16 cursor_max_x() const;
    u16 cursor_max_y() const;

    // Dirty-cell size (16px resolve_cell coarsens only if the surface outgrows GridCapacity).
    u16 cell_size() const { return m_cell_size; }

    // Cell counts rounded up per axis.
    u16 grid_cols() const { return m_grid_cols; }
    u16 grid_rows() const { return m_grid_rows; }
    u32 grid_bytes() const { return u32(m_grid_cols) * m_grid_rows; }

    // World scratch buffer (g_game_map_data_for_blit) covering world + 1 tile margin per axis for scroll wrapping.
    u32 map_pitch() const { return m_map_pitch; }
    u32 map_rows() const { return m_map_rows; }
    u32 map_size() const { return m_map_size; }

    u16 map_tile_cols() const { return m_map_tile_cols; }
    u16 map_tile_rows() const { return m_map_tile_rows; }

    // Megatile mask arrays at 006D5C0C/10/14 (1 byte per terrain tile, +1 tile border for blur neighbour reads in 0047FC50 / 004804D0).
    u16 mask_stride() const;
    u16 mask_rows() const;
    u32 mask_bytes() const;

    // Fog visibility array at 006D5C18 (4 minitiles per terrain tile axis; 004805F0).
    u16 vision_stride() const;
    u16 vision_rows() const;
    u32 vision_bytes() const;

    // Full-FOV (100% floor) sizes for buffers allocated before the game's role is
    // known: the world extent only ever shrinks below these for player-restricted
    // roles, and a witness role resolved later than allocation can grow into them.
    u32 full_mask_bytes() const;
    u32 full_vision_bytes() const;

private:
    // Recomputes the floor-derived extents (world size and the scratch-buffer chain
    // built on top of it). The getters above sit in per-tile/per-sprite hot paths, so
    // the derived sizes are cached members rather than recomputed division chains.
    void recompute_world_extents();

    u16 m_surface_width = display::LegacyWidth;
    u16 m_surface_height = display::LegacyHeight;
    u16 m_cell_size = display::MinCellSize;
    u16 m_grid_cols = display::cells_across(display::LegacyWidth, display::MinCellSize);
    u16 m_grid_rows = display::cells_across(display::LegacyHeight, display::MinCellSize);
    u32 m_min_zoom_percent = display::MinZoomPercent;

    // Defaults mirror the legacy surface at the 100% floor: pre-init getter values
    // match what the former computed getters returned.
    u16 m_world_width = display::LegacyWidth;
    u16 m_world_height = display::LegacyHeight;
    u32 m_map_pitch = display::map_axis(display::LegacyWidth);
    u32 m_map_rows = display::map_axis(display::LegacyHeight);
    u32 m_map_size = m_map_pitch * m_map_rows;
    u16 m_map_tile_cols = u16(m_map_pitch / display::TileSize);
    u16 m_map_tile_rows = u16(m_map_rows / display::TileSize);
};

inline DisplayGeometry g_display;

struct DirtyCellGrids {
    std::unique_ptr<u8[]> surface;
    std::unique_ptr<u8[]> world;
    void init();
};

inline DirtyCellGrids g_dirty_cell_grids;

// Resolves the replay/witness role, applies its zoom floor to the display
// geometry (and with it the rendered world extent), and resets the zoom to the
// default. Must run before map-load allocations read g_display
// (called from the alloc_fog_arrays hook).
void apply_display_role();
