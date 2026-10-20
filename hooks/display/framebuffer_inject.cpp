#include "framebuffer.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../graphics/Bitmap.h"
#include "../../storm/storm.h"
#include "../../hooks/limits/sfx_extender.h"

namespace {
// DirectDraw init (0041D930) dimensions:
// - 0041DA3E / 0041DA43: push height / width for SetDisplayMode(w, h, 8) at 0041DA48
// - 0041DB9F / 0041DBA6: DDSURFACEDESC dwWidth / dwHeight for offscreen surface
// - 1505A7C4 / 1505A7C8: storm.dll#351 screen dimensions (read by storm.dll#342 screenshots)
void patch_ddraw_dimensions() {
    if (!g_display.is_widescreen()) {
        return;
    }

    const u32 width = g_display.surface_width();
    const u32 height = g_display.surface_height();

    memory_patch(0x0041DA3E, height);
    memory_patch(0x0041DA43, width);
    memory_patch(0x0041DB9F, width);
    memory_patch(0x0041DBA6, height);

    memory_patch(0x1505A7C4, width);
    memory_patch(0x1505A7C8, height);
}

} // namespace

namespace hooks {

void inject_framebuffer_hooks() {
    patch_ddraw_dimensions();
    repair_storm_surface_copy_procs();

    jump_patch(init_screen_state, 0x0041E050);
    jump_patch(graphics_init, 0x004DB060);
    jump_patch(graphics_shutdown, 0x004DA310);
}

} // hooks
