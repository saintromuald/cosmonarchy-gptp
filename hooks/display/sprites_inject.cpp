#include "sprites.h"
#include "../../hook_tools.h"

// In 004D57B0 (calculates GRP image screen clip rectangle), patches viewport limits
// (legacy 640x400). Patched in-place because isdbg.qdp call-patches 004D57B6.
namespace hooks {

void patch_image_clip() {
    // Vanilla world extent: 640x480 minus its 80 reserved HUD rows.
    if (g_display.world_width() == display::LegacyWidth &&
        g_display.world_height() == display::LegacyWorldHeight) {
        return;
    }

    // 004D5857: operand of `mov esi, 0x280` (viewport width clip).
    // 004D5888: operand of `mov ecx, 0x190` (viewport height clip).
    memory_patch(0x004D5857, u32(g_display.world_width()));
    memory_patch(0x004D5888, u32(g_display.world_height()));
}

void inject_sprite_hooks() {
    patch_image_clip();
}

} // hooks