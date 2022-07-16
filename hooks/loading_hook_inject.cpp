#include "loading_hook.h"
#include <hook_tools.h>

const u32 Hook_loadingDataReturn = 0x004DB056;
void __declspec(naked) loading_data_hook() {
    __asm
    {
        PUSHAD
    }
    hooks::loading_data();
    __asm
    {
        POPAD
        RETN
    }
}

namespace hooks {
    void inject_loading_hook() {
        jump_patch(loading_data_hook, Hook_loadingDataReturn);
    }
}