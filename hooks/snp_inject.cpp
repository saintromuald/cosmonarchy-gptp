#include "snp.h"
#include "hook_tools.h"

namespace hooks {
void inject_snp_hooks() {
    call_patch(glu_connect_init_provider, 0x004bae93);
    jump_patch(glu_destroy_provider, 0x00410112);
}
}