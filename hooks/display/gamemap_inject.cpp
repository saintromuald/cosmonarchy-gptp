#include "gamemap.h"
#include "../../hook_tools.h"

// Patches world scratch buffer allocation size in 004BD6F0 (g_game_map_data_for_blit,
// legacy 0x49800 = 672x448). Patched in-place because aise.qdp patches 004BD72C.
namespace {

void patch_map_buffer_size() {
    // Allocated once per process, before the replay/witness role is known, so the
    // immediate is patched for the full-FOV extent (the largest world any role can
    // render). Role-restricted players logically use a smaller wrap size within it;
    // all wrap math reads g_display at draw time.
    const u32 size = display::map_axis(g_display.display_width()) * display::map_axis(g_display.display_height());
    if (size == display::LegacyMapSize) {
        return;
    }

    // 004BD745: operand of `push 0x49800` (passed to SMemAlloc).
    memory_patch(0x004BD746, size);
}

} // namespace

namespace hooks {

void inject_gamemap_hooks() {
    patch_map_buffer_size();
}

} // hooks