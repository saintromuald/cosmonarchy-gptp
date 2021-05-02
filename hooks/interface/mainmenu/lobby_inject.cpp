#include "lobby.h"
#include <hook_tools.h>

namespace
{ // Wrappers

    void __declspec(naked) initGame_Wrapper()
    { // 004EE110
        static s32 result;
        __asm {
            PUSHAD
        }
        result = hooks::initGame();
        __asm {
            POPAD
            MOV result, EAX
            RETN
        }
    }

    void __declspec(naked) startGame_Wrapper()
    { // 00472060
        static s32 result;
        static void* a1;
        static s32 a2;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV a2, EAX
            POP EAX
            MOV a1, EAX
            PUSHAD
        }
        result = hooks::startGame(a1, a2);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 4
        }
    }

}

namespace hooks
{

    void injectLobbyHooks()
    {
        jmpPatch(initGame_Wrapper, 0x004EE110, 1);
        jmpPatch(startGame_Wrapper, 0x00472060, 4);
    }
}