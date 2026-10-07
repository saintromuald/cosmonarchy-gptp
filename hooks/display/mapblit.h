#pragma once
#include "../../systems/display/display.h"

namespace hooks {

void __cdecl blit_map_rows(s32 src_offset, s32 rows, const u8* src, u8* dest);  // 0040C200
void __cdecl blit_map_cell_row(s32 y, s32 x, s32 width, s32 src_offset);        // 0040C2BD
void __cdecl blit_map_viewport();                                               // 0040C253

void inject_mapblit_hooks();

} // hooks
