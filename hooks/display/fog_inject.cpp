#include "fog.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../storm/storm.h"

namespace {
void __declspec(naked) alloc_fog_arrays_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::alloc_fog_arrays();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) free_fog_arrays_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::free_fog_arrays();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) build_fog_mask_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::build_fog_mask();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) build_fog_vision_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::build_fog_vision();
    __asm {
        POPAD
        RETN
    }
}

void __declspec(naked) mark_fog_changes_wrapper() {
    __asm {
        PUSHAD
    }
    hooks::mark_fog_changes();
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_fog_hooks() {
    jump_patch(build_fog_mask_wrapper, 0x0047FC50);
    jump_patch(build_fog_vision_wrapper, 0x0047FE10);
    jump_patch(free_fog_arrays_wrapper, 0x00480350);
    jump_patch(mark_fog_changes_wrapper, 0x004804D0);
    jump_patch(alloc_fog_arrays_wrapper, 0x00480960);
}

} // hooks
