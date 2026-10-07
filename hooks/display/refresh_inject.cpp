#include "refresh.h"
#include "../../hook_tools.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"

namespace {
// 004805F0(edx = top, eax = bottom, [esp+4] = left, [esp+8] = right),
// callee-clean. EAX is captured before it is used to read the stack.
void __declspec(naked) apply_fog_wrapper() {
    static s32 top;
    static s32 bottom;
    static s32 left;
    static s32 right;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV top, EDX
        MOV bottom, EAX
        MOV EAX, [EBP + 0x08]
        MOV left, EAX
        MOV EAX, [EBP + 0x0C]
        MOV right, EAX
        PUSHAD
    }
    hooks::apply_fog(top, bottom, left, right);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 0x08
    }
}

// 004808E0 takes its "refresh everything" flag in eax and returns nothing.
void __declspec(naked) refresh_dirty_cells_wrapper() {
    static u32 refresh_all;
    __asm {
        MOV refresh_all, EAX
        PUSHAD
    }
    hooks::refresh_dirty_cells(refresh_all);
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_refresh_hooks() {
    jump_patch(refresh_dirty_cells_wrapper, 0x004808E0);
    jump_patch(apply_fog_wrapper, 0x004805F0);
}

} // hooks
