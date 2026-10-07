#include "frame_pacing.h"
#include "../../hook_tools.h"

namespace hooks {

void inject_frame_pacing_hooks() {
    // begin_game calls it and returns to the glue when it does; a plain cdecl
    // replacement at its entry keeps that. The 5-byte jump ends inside the
    // original's `sub esp, 0xC`, hence the trailing nop.
    jump_patch(game_loop_top, 0x004D9840, 1);
}

} // hooks
