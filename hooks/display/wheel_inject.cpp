#include "wheel.h"
#include "../../hook_tools.h"

namespace {

// 004D1900(eax = event type, ecx = packed position, edi = input proc), ret.
// Neither caller reads a register back - main_window_proc sets eax to 1 and
// returns - so everything the original clobbered can be preserved, exactly as
// the button dispatchers next door are wrapped.
void __declspec(naked) mouse_wheel_wrapper() {
    static u32 event_type;
    static u32 packed_position;
    static InputProcsFunc* input_proc;
    __asm {
        MOV event_type, EAX
        MOV packed_position, ECX
        MOV input_proc, EDI
        PUSHAD
    }
    hooks::mouse_wheel(event_type, packed_position, input_proc);
    __asm {
        POPAD
        RETN
    }
}

} // namespace

namespace hooks {

void inject_wheel_hooks() {
    jump_patch(mouse_wheel_wrapper, 0x004D1900);
}

} // hooks
