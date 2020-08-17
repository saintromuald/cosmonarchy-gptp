//
#include "hooks/misc/hydra_specific.h"

namespace hooks {
    const u32 jmp_00494FC3 = 0x00494FC3;
    const u32 jmp_00494FAC = 0x00494FAC;
    void __declspec(naked) getFlingyHaltDistance_94F9E_JmpWrapper() {
        //patched 2 instructions before 94fa3 to avoid aise extender conflict
        static u32 current_speed;
        static u16 flingy_id;
        static CFlingy* flingy;
        __asm {
            MOVZX EDX, [ECX + 0x24]
            PUSH ESI
            MOV current_speed, EAX
            MOV flingy_id, DX
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
            PUSH EBX
            XOR EBX,EBX
            MOV[ESI + 0x3C],EBX
            POP EBX
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
    void flingySpeedPatches() {//for madrigals
        jmpPatch(getFlingyHaltDistance_94F9E_JmpWrapper, 0x00494F9E);
        jmpPatch(initializeFlingy_96377_JmpWrapper, 0x00496377, 2);
    }

    const u32 jmp_004295F8 = 0x004295F8;
    const u32 jmp_004295F3 = 0x004295F3;
    void __declspec(naked) wrapper_295EB() {
        static u16 id;
        __asm {
            MOV id,DI
            PUSHAD
        }
        if (!hooks::is_high_templar_conduit(id)) {
            __asm {
                POPAD
                MOV AL,0x6A
                JMP jmp_004295F8
            }
        }
        else {
            __asm {
                POPAD
                MOV AL, 0x6A
                JMP jmp_004295F3
            }
        }
 
    }

    void archonHardcodePatches(){//for augurs
        jmpPatch(wrapper_295EB, 0x004295EB, 1);
    }
    void injectHydraSpecific() {
        flingySpeedPatches();
        archonHardcodePatches();
    }
}

//all archon hardcode cases (copied from Overlord):
/*
    memoryPatch(0x004295EB + 3, upperLimit, sizeof upperLimit);//+ //DONE
    memoryPatch(0x00435E11 + 2, upperLimit, sizeof upperLimit);//+ (ai)
    memoryPatch(0x00435F4D + 2, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x00442915 + 3, upperLimit, sizeof upperLimit);
    memoryPatch(0x00447D37 + 2, upperLimit, sizeof upperLimit);
    memoryPatch(0x004484B8 + 2, upperLimit, sizeof upperLimit);
    memoryPatch(0x00455739 + 3, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x0046DD32 + 3, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004C115E + 2, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004C134E + 2, upperLimit, sizeof upperLimit);
    memoryPatch(0x004C153E + 2, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004C1763 + 3, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004C1DC8 + 2, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004C227D + 2, upperLimit, sizeof upperLimit);//+
    memoryPatch(0x004CC55D + 3, upperLimit, sizeof upperLimit);//+
*/