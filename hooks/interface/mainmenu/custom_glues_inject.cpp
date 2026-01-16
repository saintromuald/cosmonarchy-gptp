#include "custom_glues.h"
#include "hook_tools.h"

const u32 glu_main_switch_de433 = 0x004de433;
static void __declspec(naked) glu_main_switch_wrapper() { // 004de445
    glu_main_switch();
    __asm {
        JMP glu_main_switch_de433
    }
}

namespace hooks {
void inject_custom_glues() {
    jump_patch(glu_main_switch_wrapper, 0x004de445);

    // Remove Sleep(1000) from glu_login cancel
    // It's not necessary with new flow to intentionally delay snetdestroy
    nop_patch(0x004c4690, 11);
}
}