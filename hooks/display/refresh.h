#pragma once
#include "../../systems/display/display.h"

namespace hooks {

void __cdecl apply_fog(s32 top, s32 bottom, s32 left, s32 right);  // 004805F0
void __cdecl refresh_dirty_cells(u32 refresh_all);                 // 004808E0

void inject_refresh_hooks();

} // hooks
