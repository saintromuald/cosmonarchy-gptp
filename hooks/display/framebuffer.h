#pragma once
#include "../../systems/display/display.h"
#include "../../graphics/Bitmap.h"

namespace hooks {

// Full-FOV world render target while magnified: the world layers composite into
// it at 1:1 and the zoom resample copies the crop onto the screen surface. A
// separate buffer keeps the crop durable across resamples - the resample's
// output covers the whole surface, so a crop living on the surface would be
// overwritten by its own magnified output (fog and stale content flicker).
// Surface-width pitch, so world-phase draws address it exactly like the screen.
inline graphics::Bitmap g_world_surface = {};

void __cdecl init_screen_state();  // 0041E050
void __cdecl graphics_init();      // 004DB060
void __cdecl graphics_shutdown();  // 004DA310

void inject_framebuffer_hooks();

} // hooks
