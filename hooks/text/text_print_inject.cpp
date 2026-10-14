#include "text_print.h"
#include "../../hook_tools.h"

namespace hooks {

// 0x004202B0: text in EAX, x in ESI, y as the one stack arg (callee pops it).
// Returns the first unprinted byte, or null when everything printed.
void __declspec(naked) bw_print_xy_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+36]     // y (the caller's stack arg)
        PUSH dword ptr [ESP+8]      // x (saved ESI)
        PUSH dword ptr [ESP+36]     // text (saved EAX)
        CALL bw_print_xy_impl
        MOV [ESP+28], EAX           // stash the result over the saved EAX
        POPAD
        RET 4
    }
}

// 0x0041FB30: the font table pointer in ECX.
void __declspec(naked) set_font_handle_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+24]     // font (saved ECX)
        CALL set_font_handle_impl
        POPAD
        RET
    }
}

void inject_text_print_hooks() {
    jump_patch(bw_print_xy_wrapper, 0x004202B0, 2);
    jump_patch(set_font_handle_wrapper, 0x0041FB30, 1);
}

} // hooks
