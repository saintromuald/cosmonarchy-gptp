#pragma once
#include "../../types.h"

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

constexpr u32 GridCapacity = 1200;
constexpr u16 MinCellSize = 16;

constexpr u16 MaxWidth = 1920;
constexpr u16 MaxHeight = 1200;

constexpr u32 LegacyMapSize = 0x49800;

// Scratch buffer axis: viewport rounded up to terrain tiles, plus one tile margin for sub-tile scrolling.
u32 map_axis(u16 view);

} // display

class DisplayGeometry {
public:
    void init();

    u16 surface_width() const;
    u16 surface_height() const;

    u16 fov_width() const;
    u16 fov_height() const;

    // Rendered world area shared by fog (004808E0), dirty-cell walker (004BCDC0), and screen copy (0040C2BD).
    u16 world_width() const;
    u16 world_height() const;

    bool is_widescreen() const;

    // The zoom output rect: where the magnified world is drawn on the surface
    // (the full surface rect).
    u16 display_width() const;
    u16 display_height() const;

    // The role's zoom floor in percent; sets the rendered world extent
    // (display / floor). Applied per game via apply_display_role() once the
    // replay/witness role is known.
    u16 min_zoom_percent() const;
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

    // 1200-byte dirty-cell grid at 006CEFF8 cleared by 0041E280.
    u8* grid() const;

    // Dirty-cell size (power of two, required by Storm 440; 16px for 640x480, 32px for 1280x720).
    u16 cell_size() const;

    // Cell counts rounded up per Storm 440.
    u16 grid_cols() const;
    u16 grid_rows() const;
    u32 grid_bytes() const;

    // World scratch buffer (g_game_map_data_for_blit) covering world + 1 tile margin per axis for scroll wrapping.
    u32 map_pitch() const;
    u32 map_rows() const;
    u32 map_size() const;

    u16 map_tile_cols() const;
    u16 map_tile_rows() const;

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
    u16 m_surface_width = display::LegacyWidth;
    u16 m_surface_height = display::LegacyHeight;
    u16 m_cell_size = display::MinCellSize;
    u32 m_min_zoom_percent = display::MinZoomPercent;
};

inline DisplayGeometry g_display;

// Resolves the replay/witness role, applies its zoom floor to the display
// geometry (and with it the rendered world extent), and resets the zoom to the
// default. Must run before map-load allocations read g_display
// (called from the alloc_fog_arrays hook).
void apply_display_role();
