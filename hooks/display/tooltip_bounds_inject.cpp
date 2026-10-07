#include "tooltip_bounds.h"

#include "../../hook_tools.h"
#include "../../systems/display/display.h"

namespace {

// Tooltip edge clamps (operand at instruction address + 1):
// - 004815E7: context help right edge clamp (00481510)
// - 00481621: context help bottom edge clamp (00481510)
// - 0045888A: button tooltip right edge clamp in 00458850 (caller 00459030 hooked by aise.qdp/mtl.qdp)
void patch_tooltip_clamps() {
    memory_patch(0x004815E7, u32(g_display.surface_width() - 1));
    memory_patch(0x00481621, u32(g_display.surface_height() - 1));
    memory_patch(0x0045888A, u32(g_display.surface_width() - 1));
}

// 00481480 parks context help offscreen when dismissed; parking at surface bounds prevents
// 0041CA00 invalidating dirty rects on screen (clipped to 0051A15C):
// - 004814F1: operand of `mov word ptr [context_layer.left], 0x280` (004814EA)
// - 004814FA: operand of `mov word ptr [context_layer.top], 0x190` (004814F3)
void patch_context_help_park() {
    memory_patch(0x004814F1, u16(g_display.surface_width()));
    memory_patch(0x004814FA, u16(g_display.surface_height()));
}

} // namespace

namespace hooks {

void inject_tooltip_bounds_hooks() {
    if (!g_display.is_widescreen()) {
        return;
    }

    patch_tooltip_clamps();
    patch_context_help_park();
}

} // hooks
