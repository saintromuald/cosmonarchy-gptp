
#include "burrow_tech.h"
#include <hook_tools.h>

namespace {
    void __declspec(naked) cmdrecv_unburrow_wrapper() {
        __asm {
            PUSHAD
        }
        hooks::cmdrecv_unburrow();
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) cmdrecv_burrow_wrapper() {
        static u8 bCommandType;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            MOV EAX, [EBP + 0x08]
            MOV AL, [EAX + 1]
            MOV bCommandType, AL
            PUSHAD
        }
        hooks::cmdrecv_burrow(bCommandType);
        __asm {
            POPAD
            POP EBP
            RETN 4
        }
    }

    void __declspec(naked) unburrow_wrapper() {
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::unburrow(unit);
        __asm {
            POPAD
            RETN
        }
    }
}

namespace hooks {
    void inject_burrow_tech_hooks() {
    //	jump_patch(cmdrecv_unburrow_wrapper,		0x004C1AC0, 3);
    //	jump_patch(cmdrecv_burrow_wrapper,			0x004C1FA0, 6);
        jump_patch(unburrow_wrapper,	0x004E97C0, 1);
    }
}