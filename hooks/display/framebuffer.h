#pragma once
#include "../../systems/display/display.h"

namespace hooks {

void __cdecl init_screen_state();  // 0041E050
void __cdecl graphics_init();      // 004DB060
void __cdecl graphics_shutdown();  // 004DA310

void inject_framebuffer_hooks();

} // hooks
