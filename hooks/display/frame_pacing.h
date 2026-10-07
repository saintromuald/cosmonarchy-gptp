#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

// 004D9840. Main in-game loop. Waits on window messages and timer deadlines via
// MsgWaitForMultipleObjectsEx, pacing presents to the display rate when dirty.
void __cdecl game_loop_top();

void inject_frame_pacing_hooks();

} // hooks
