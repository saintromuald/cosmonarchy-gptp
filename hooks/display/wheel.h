#pragma once
#include "../../BW/bwdata.h"

namespace hooks {

// 004D1900(eax = event type, ecx = packed position, edi = input proc), ret.
// Dispatches mouse wheel events (0x11/0x12 from 004D2487) to zoom, falling back
// to dialogs and input_proc.
void __cdecl mouse_wheel(u32 event_type, u32 packed_position, InputProcsFunc* input_proc);

void inject_wheel_hooks();

} // hooks
