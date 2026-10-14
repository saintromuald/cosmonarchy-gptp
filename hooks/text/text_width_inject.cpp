#include "text_width.h"
#include "../../hook_tools.h"

namespace hooks {

// 0x0041F920: text in EAX; returns the break pointer.
void __declspec(naked) font_find_line_break_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+28]     // text (saved EAX)
        CALL font_find_line_break_impl
        MOV [ESP+28], EAX
        POPAD
        RET
    }
}

// 0x0041FC20: text in EAX, max width as the one stack arg (callee pops it).
void __declspec(naked) font_fit_text_to_width_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+36]     // max_width (the caller's stack arg)
        PUSH dword ptr [ESP+32]     // text (saved EAX)
        CALL font_fit_text_to_width_impl
        MOV [ESP+28], EAX
        POPAD
        RET 4
    }
}

// 0x0041FC80: text in EAX; returns the widest line's width.
void __declspec(naked) font_max_line_width_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+28]     // text (saved EAX)
        CALL font_max_line_width_impl
        MOV [ESP+28], EAX
        POPAD
        RET
    }
}

// 0x00416240: the dialog in ESI.
void __declspec(naked) textbox_scroll_to_cursor_wrapper() {
    __asm {
        PUSHAD
        PUSH dword ptr [ESP+4]      // dlg (saved ESI)
        CALL textbox_scroll_to_cursor_impl
        POPAD
        RET
    }
}

void inject_text_width_hooks() {
    jump_patch(font_find_line_break_wrapper, 0x0041F920, 1);
    jump_patch(font_fit_text_to_width_wrapper, 0x0041FC20);
    jump_patch(font_max_line_width_wrapper, 0x0041FC80, 1);
    jump_patch(textbox_scroll_to_cursor_wrapper, 0x00416240, 3);
}

} // hooks
