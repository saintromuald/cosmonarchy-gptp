#include "starfield.h"
#include "../../hook_tools.h"

namespace hooks {

// Both walkers are cdecl void() with no arguments, so the C++ replacements patch in directly
// (same pattern as grid_inject).
void inject_starfield_hooks() {
    jump_patch(draw_starfield, 0x0047EE20);
    jump_patch(draw_starfield_wrapped, 0x0047EBF0);
}

} // hooks