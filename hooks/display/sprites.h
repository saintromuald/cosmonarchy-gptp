#pragma once
#include "../../systems/display/display.h"

namespace hooks {

// Patches 004D57B0's GRP image clip immediates (004D5857 / 004D5888) with the
// current rendered world extent. Applied per game (map load and first frame),
// because the role-based extent is only known then.
void patch_image_clip();

void inject_sprite_hooks();

} // hooks