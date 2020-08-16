//
#include "hooks/misc/hydra_specific.h"

namespace hooks {
    const u32 jmp_00494FC3 = 0x00494FC3;
    const u32 jmp_00494FAC = 0x00494FAC;
    void __declspec(naked) getFlingyHaltDistance_94F9E_JmpWrapper() {
        //patched 2 instructions before 94fa3 to avoid aise extender conflict
        static u32 current_speed;
        static u32 flingy_id;
        static CFlingy* flingy;
        __asm {
            MOVZX EDX, [ECX + 0x24]
            PUSH ESI
            MOV current_speed, EAX
            MOV flingy_id, EDX
            MOV flingy, ECX
            PUSHAD
        }
        if (hooks::realTopSpeed(flingy, flingy_id) != current_speed) {
            __asm {
                POPAD
                JMP jmp_00494FC3
            }
        }
        else {
            __asm {
                POPAD
                JMP jmp_00494FAC
            }
        }
    }

    const u32 jmp_00496385 = 0x00496385;
    void __declspec(naked) initializeFlingy_96377_JmpWrapper() {
        static u32 topSpeed;
        static u32 flingy_id;
        static CFlingy* flingy;
        __asm {
            MOV[ESI + 0x3C], 0
            MOV flingy_id, EAX
            MOV flingy, ESI
            PUSHAD
        }
        topSpeed = hooks::realTopSpeed(flingy, flingy_id);
        __asm {
            POPAD
            MOV ECX, topSpeed
            JMP jmp_00496385
        }
    }
    void injectHydraSpecific() {
        jmpPatch(getFlingyHaltDistance_94F9E_JmpWrapper, 0x00494F9E);
        jmpPatch(initializeFlingy_96377_JmpWrapper, 0x00494F9E,2);
    }
}
