#pragma once
#include "../../systems/display/display.h"

namespace hooks {

void __cdecl alloc_fog_arrays();  // 00480960
void __cdecl free_fog_arrays();   // 00480350
void __cdecl build_fog_mask();    // 0047FC50
void __cdecl build_fog_vision();  // 0047FE10
void __cdecl mark_fog_changes();  // 004804D0

void inject_fog_hooks();

} // hooks
