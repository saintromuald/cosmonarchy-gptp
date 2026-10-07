#pragma once

#include "../../BW/structures.h"
#include "../../BW/structures/ui_structures.h"

namespace hooks {

// 004EF560 generic_image_update (DialogControlType::Image). Args: (dlg, src_x, src_y, clip).
// Bypasses Storm's ~640px staging buffer for translucent images; opaque images use vanilla copy.
u32 __fastcall image_draw(bw::Dialog* dlg, s32 source_x, s32 source_y, const Box16* clip);

void inject_translucent_image_hooks();

} // hooks
