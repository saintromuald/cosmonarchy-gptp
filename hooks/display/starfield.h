#pragma once

namespace hooks {

void __cdecl draw_starfield();          // 0047EE20
void __cdecl draw_starfield_wrapped();  // 0047EBF0

void inject_starfield_hooks();

} // hooks