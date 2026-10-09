#include "camera.h"
#include "../../BW/bwdata.h"

#include <algorithm>

// Center-view callers subtract half-viewport before calling move_screen (0049C440).
// Zoom-aware callers invert the edge-aware crop slide via g_zoom.
// - 004E6020: Ctrl+C / portrait centering (hooks::move_screen_to_unit, ui_features.cpp)
// - 004C6E68: Center View trigger action (immediates re-applied per role, camera_inject.cpp)
// - 0045E3A3: Remember last event position (immediates re-applied per role, camera_inject.cpp)
// - 00484460: Middle mouse drag (ui_features.cpp)
// - 004A4D61: Minimap click and drag (minimap.cpp)
// - 004967E0: Control group centering (ControlGroup::center_camera, structures.cpp)
// - bw::move_camera: g_zoom.camera_for_center
//
// 0049BB90: Sets scroll clamps (g_move_to_x_max, g_move_to_y_max) from map size at load time.
namespace hooks {

// Re-derives the scroll clamps from the current map size and rendered world
// extent, clamping the camera into range. The clamps depend on the display
// role's world extent, so this runs wherever the role is (re)applied — the
// load sequence may compute them before the extent is final (e.g. replay role
// applies at fog-alloc), leaving them one extent stale and letting the camera
// scroll past the right/bottom map edges.
void update_scroll_limits() {
    const s32 width_pixels = s32(g_map_tile_size->width) * display::TileSize;
    const s32 height_pixels = s32(g_map_tile_size->height) * display::TileSize;

    g_move_to_x_max = g_display.scroll_max_x(width_pixels);
    g_move_to_y_max = g_display.scroll_max_y(height_pixels);
    g_move_to_x = std::min(g_move_to_x, g_move_to_x_max);
    g_move_to_y = std::min(g_move_to_y, g_move_to_y_max);
}

void __cdecl set_camera_map_bounds() { // 0049BB90
    const s32 width_pixels = s32(g_map_tile_size->width) * display::TileSize;
    const s32 height_pixels = s32(g_map_tile_size->height) * display::TileSize;

    g_map_width_pixels = s16(width_pixels);
    g_map_height_pixels = s16(height_pixels);
    g_full_map_pixel_dimensions.left = 0;
    g_full_map_pixel_dimensions.top = 0;
    g_full_map_pixel_dimensions.right = s16(width_pixels);
    g_full_map_pixel_dimensions.bottom = s16(height_pixels);

    update_scroll_limits();

    g_move_to_x = s32(g_move_to_tile_x) * display::TileSize;
    g_move_to_y = s32(g_move_to_tile_y) * display::TileSize;
}

} // hooks
