#include "screen_text.h"
#include "../../hook_tools.h"

namespace hooks {

static void __declspec(naked) display_text_message_wrapper() { // 0x0048CD30
    static const char* text;
    static u16 packed_color;
    static u32 ticks;
    static u8 from_trigger;
    __asm {
        PUSHAD
        MOV EAX, [ESP+28]     // text (saved EAX)
        MOV text, EAX
        MOV EAX, [ESP+36]
        MOV packed_color, AX
        MOV EAX, [ESP+40]
        MOV ticks, EAX
        MOV EAX, [ESP+44]
        MOV from_trigger, AL
    }
    display_text_message(text, packed_color, ticks, from_trigger);
    __asm {
        POPAD
        RET 12
    }
}

void inject_screen_text_hooks() {
    // The call site rather than the function, because the deferred half has to
    // reach the original body afterwards. 0048CF60 has exactly one caller.
    call_patch(draw_screen_text, 0x004BD614);
    jump_patch(init_screen_text_layout, 0x0048CE90, 1);
    jump_patch(display_text_message_wrapper, 0x0048CD30, 1);
}

} // hooks