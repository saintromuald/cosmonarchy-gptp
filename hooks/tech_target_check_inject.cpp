#include "tech_target_check.h"
#include <hook_tools.h>

namespace {
void __declspec(naked) get_tech_use_error_message_hook_wrapper() { // 00491E80
    static Unit* target;
    static u8 player;
    static u32 tech_id;
    static u32 result;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSH EAX
        MOV EAX, [EBP + 0x08]
        MOV tech_id, EAX
        POP EAX
        MOV target, EAX
        MOV player, BL
        PUSHAD
    }
    result = hooks::get_tech_use_error_message(target, player, tech_id);
    __asm {
        POPAD
        MOV EAX, result
        POP EBP
        RETN 0x04
    }
}
}

namespace hooks {
void inject_tech_target_check_hooks() {
    jump_patch(get_tech_use_error_message_hook_wrapper, 0x00491E80, 4);
}
}