#include "dialog_clip.h"

#include "../../hook_tools.h"
#include "../../systems/display/display.h"

namespace {

// Both boxes are four dwords: left, top, right, bottom.
void patch_dialog_clip_box(u32 address, s32 right, s32 bottom) {
    const s32 box[4] = { 0, 0, right, bottom };
    memory_patch(address, (const u8*)box, sizeof(box));
}

} // namespace

namespace hooks {

void inject_dialog_clip_hooks() {
    if (!g_display.is_widescreen()) {
        return;
    }

    const s32 width = s32(g_display.surface_width());
    const s32 height = s32(g_display.surface_height());

    patch_dialog_clip_box(0x0051A15C, width - 1, height - 1);
    patch_dialog_clip_box(0x0051A16C, width, height);
}

} // hooks
