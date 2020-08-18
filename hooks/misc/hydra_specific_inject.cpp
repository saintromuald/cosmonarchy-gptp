//
#include "hooks/misc/hydra_specific.h"
#include "globals.h"
namespace hooks {

    void __declspec(naked) getFlingyHaltDistanceHookWrapper() {
        static CFlingy* flingy;
        static u32 result;
        __asm {
            MOV flingy,ECX
            PUSHAD
        }
        result = hooks::getFlingyHaltDistance(flingy);
        __asm {
            POPAD
            MOV EAX,result
            RETN
        }
    }

    const u32 jmp_00496390 = 0x00496390;
    void __declspec(naked) initializeFlingy_96377_JmpWrapper() {
        static u32 topSpeed;
        static u32 flingy_id;
        static u16 acceleration;
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
        acceleration = hooks::realAcceleration(flingy, flingy_id);
        Globals.flingy_init_bullet_id = -1;
        __asm {
            POPAD
            MOV ECX, topSpeed
            MOV [ESI+0x34], ECX
            MOV CX, acceleration
            JMP jmp_00496390
        }
    }
    const u32 jmp_8BEDE = 0x0048BEDE;
    void __declspec(naked) initializeBullet_8BED7_JmpWrapper() {
        static u32 weapon_id;
        static u32 player_id;
        static u32 result;
        __asm {
            MOV weapon_id,EAX
            MOV player_id,EBX
            PUSHAD
        }
        result = weapon_flingy(weapon_id, player_id);
        __asm {
            POPAD
            MOV EAX, result
            JMP jmp_8BEDE;
        }
    }

    void flingySpeedPatches() {//for madrigals
        jmpPatch(getFlingyHaltDistanceHookWrapper, 0x00494F90);
        jmpPatch(initializeFlingy_96377_JmpWrapper, 0x00496377, 2);
        jmpPatch(initializeBullet_8BED7_JmpWrapper, 0x0048BED7, 2);
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
    const u32 func_getTargetSomething = 0x0043FF90;
    const u32 progressOrderSwitch_e71b = 0x004EC71B;
    const u32 spellOrderCases = 0x004ECADB;
    void __declspec(naked) unusedOrderPatchWrapper() {
        
        static u8 orderId;
        __asm {
            MOV [ESI+0x85], 0x8
            CALL func_getTargetSomething
            MOV AL,[ESI+0x4D]
            MOV orderId, AL
            MOVZX EAX,AL
            PUSHAD
        }
        if (hooks::is_unused_order(orderId)) {
            __asm {
                POPAD
                JMP spellOrderCases
            }
        }
        else {
            __asm {
                POPAD
                JMP progressOrderSwitch_e71b
            }
        }
    }
    void unusedOrderPatches() {
        jmpPatch(unusedOrderPatchWrapper, 0x004EC70B, 7);//turns unused orders to spell orders
        //later add jump patches to make CTFCop2 and WarpIn useless
    }
    void archonHardcodePatches(){//for augurs
        jmpPatch(wrapper_295EB, 0x004295EB, 1);
    }
    void injectHydraSpecific() {
        flingySpeedPatches();
        //start of spell order code
        //default case of switch 004EC4FF
        unusedOrderPatches();
        //disables CTFCop2 functionality, makes it useless
        //archonHardcodePatches();
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