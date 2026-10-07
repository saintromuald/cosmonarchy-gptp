#pragma once
#include "../../systems/display/display.h"

namespace hooks {

// Re-derives scroll clamps from the current world extent and zoom; called
// wherever the display role is (re)applied (see refresh_display_role) and
// whenever a zoom transition moves the console overscroll allowance.
void update_scroll_limits();

// Re-applies the half-viewport immediates baked into vanilla's centering paths
// (Center View trigger action, last-event writer, Ctrl+C follow camera). The
// offsets derive from the role's world extent, so this must run wherever the
// display role is (re)applied; the attach-time values bake the player floor.
void patch_camera_center_immediates();

void __cdecl set_camera_map_bounds();  // 0049BB90

void inject_camera_hooks();

} // hooks
