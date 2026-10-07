#include "screen_text.h"
#include "../../hook_tools.h"

namespace hooks {

void inject_screen_text_hooks() {
    // The call site rather than the function, because the deferred half has to
    // reach the original body afterwards. 0048CF60 has exactly one caller.
    call_patch(draw_screen_text, 0x004BD614);
    jump_patch(init_screen_text_layout, 0x0048CE90, 1);
}

} // hooks