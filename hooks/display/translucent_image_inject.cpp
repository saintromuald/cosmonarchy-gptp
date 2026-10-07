#include "translucent_image.h"
#include "../../hook_tools.h"

namespace hooks {

void inject_translucent_image_hooks() {
    jump_patch(image_draw, 0x004EF560);
}

} // hooks
