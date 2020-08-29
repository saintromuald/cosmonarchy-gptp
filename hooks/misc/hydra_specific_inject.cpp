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
    const u32 jmp_004A0682 = 0x004A0682;
    void __declspec(naked) lobotomyMineJmpWrapper() {
        CUnit* unit;
        __asm {
            MOV unit,ESI
            PUSHAD
        }
        setupSpiderMines(unit);
        __asm {
            POPAD
            JMP jmp_004A0682
        }

    }

    const u32 jmp_0046512B = 0x0046512B;
    void orders_PlaceMine_JmpWrapper() {
        CUnit* unit;
        u32 result;
        __asm {
            MOV unit,ESI
            PUSHAD
        }
        result = hooks::get_mine_id(unit);
        __asm {
            POPAD
            MOV EAX,result
            JMP jmp_0046512B
        }
    }

    const u32 jmp_0046514A = 0x0046514A;
    void orders_PlaceMine_JmpWrapper2() {
        CUnit* unit;
        u32 result;
        __asm {
            MOV unit, ESI
            PUSHAD
        }
        result = hooks::get_mine_id(unit);
        __asm {
            POPAD
            MOV ECX, result
            JMP jmp_0046514A
        }
    }

    void unusedOrderPatches() {
        jmpPatch(unusedOrderPatchWrapper, 0x004EC70B, 7);//turns unused orders to spell orders
        //later add jump patches to make CTFCop2 and WarpIn useless
    }
    void archonHardcodePatches(){//for augurs
        jmpPatch(wrapper_295EB, 0x004295EB, 1);
    }

  
    void lobotomyMinePatches() {
      
        jmpPatch(lobotomyMineJmpWrapper, 0x004A066B, 3);  /*
        jmpPatch(orders_PlaceMine_JmpWrapper, 0x00465126, 0);
        jmpPatch(orders_PlaceMine_JmpWrapper2, 0x00465145, 0);
        nops(0x00465088, 8);//remove tech requirement for any mines */
    }
    const u32 func_techTargetSpellCheck = 0x00491E80;
    const u32 jmp_004920E3 = 0x004920E3;
    void __declspec(naked) getOrderTargetErrorString_JmpWrapper() {
        static CUnit* caster;
        static u8 playerId;
        static u32 techId;
        static CUnit* target;
        static u32 func_result;
        static u32 extended_result;
        __asm {
            MOV caster,EAX
            MOV BL,[EAX+0x4c]   //
            MOV EAX,EDI //
            MOV target,EAX
            MOV playerId,BL
            CALL func_techTargetSpellCheck
            MOV func_result, EAX
            PUSHAD
        }
        extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
        __asm {
            POPAD
            MOV EAX,extended_result
            JMP jmp_004920E3
        }
    }
    const u32 jmp_004922E1 = 0x004922E1;
    void __declspec(naked) ShouldStopOrderedSpell_JmpWrapper() {
        static CUnit* caster;
        static u8 playerId;
        static u32 techId;
        static CUnit* target;
        static u32 func_result;
        static u32 extended_result;
        __asm {
            PUSH EAX
            MOV EAX,[EBP+0x08]
            MOV techId,EAX
            POP EAX
            MOV playerId,BL
            MOV target,EAX
            MOV caster,EDI
            CALL func_techTargetSpellCheck
            MOV func_result,EAX
            PUSHAD
        }
        extended_result = hooks::extend_tech_target_check(func_result, caster, target, techId);
        __asm {
            POPAD
            MOV EAX, extended_result
            JMP jmp_004922E1
        }
    }
    void lobotomyMineTargetingPatches() {
        jmpPatch(getOrderTargetErrorString_JmpWrapper, 0x004920D9);
        jmpPatch(ShouldStopOrderedSpell_JmpWrapper, 0x004922DC);
    }
    void trapJumpWrappers() {
        memoryPatch(0x00401438, (u8)0xEB);//change to jump
        u8 patchb030[] = {0x33, 0xc0, 0x90, 0x90};
        memoryPatch(0x0047B030, patchb030);//xor eax, eax; nop; nop
        memoryPatch(0x0047B070, patchb030);//xor eax, eax; nop; nop
        //
        memoryPatch(0x004EBE99, (u8)0xEB);
        memoryPatch(0x004A4B4D, (u8)0xEB);
        memoryPatch(0x004A491B, (u8)0xEB);
        memoryPatch(0x004A4829, (u8)0xEB);
        memoryPatch(0x004A46CA, (u8)0xEB);
        memoryPatch(0x004A0307, (u8)0xEB);
        memoryPatch(0x004A3E04, (u8)0xEB);
        //
        u8 jmpp[] = {0xEB, 0x3d};
        memoryPatch(0x0049FAEF, jmpp);

//        memoryPatch(0x0049FAEF, (u8)0x75);//JNZ
        //
        memoryPatch(0x0047C05D+2, (u8)0xe4);//floor gun trap
        memoryPatch(0x0047C069+2, (u8)0xe4);//floor missile trap
        

        u8 jmpp2[] = { 0xeb,0x2c };
        memoryPatch(0x0047BEE7, jmpp2);//JNZ

        memoryPatch(0x0047B516, patchb030);
        u8 jmpToEnd[] = { 0xC3,0x90,0x90,0x90};//RETN, NOP (3)
        memoryPatch(0x0047B556, jmpToEnd);
        //
        memoryPatch(0x0047B7B1 + 2, (u8)0xE4);
        nops(0x0047B7B7, 6);
        //
        //0047BDCB
        u8 jmpp3[] = { 0xeb,0x25 };
        memoryPatch(0x0047BDCB, jmpp3);//JNZ
        

        //


        /*
        
	Line 184719: 0047BDC7   .  66:3D CB00    CMP AX,0CB
        */
    }
    const u32 progressOrder_jmp = 0x004EC6F1;
    void __declspec(naked) PhaseLinkOrder_Wrapper() {
        static CUnit* unit;
        __asm {
            MOV unit,ESI
            PUSHAD
        }
        hooks::order_PhaseLink(unit);
        __asm {
            POPAD
            JMP progressOrder_jmp
        }
    }
    void phaseLinkPatches() {   
        jmpPatch(PhaseLinkOrder_Wrapper, 0x004EC629, 2);
    }
    void __declspec(naked) canAttackTarget_Tail_Wrapper() {
        static u32 target_flags;
        static CUnit* source;
        static u32 result;
        __asm {
            PUSH EAX
            MOV EAX,[EBP-0x4]
            MOV target_flags,EAX
            POP EAX
            MOV source,EDI
            PUSHAD
        }
        result = hooks::canAttackTarget_EndChunk(source,target_flags);
        __asm {
            POPAD
            MOV EAX,result
            POP EDI
            MOV ESP,EBP
            POP EBP
            RETN 4
        }
    }
    void __declspec(naked) getGroundWeaponHookWrapper() {
        static CUnit* unit;
        u32 result;
        __asm {
            MOV unit,ECX
            PUSHAD
        }
        result = unit->getGroundWeapon();
        __asm {
            POPAD
            MOV EAX,result
            RETN
        }
    }
    void clarionWrappers() {
        //canAttackTarget - end
        jmpPatch(canAttackTarget_Tail_Wrapper, 0x0047682C, 1);
        //tail wrapper disabled, without air weapon it don't have sense anywhere, will be reworked w/ air target functionality
        //jmpPatch(getGroundWeaponHookWrapper, 0x00475AD0);
    }
    void injectHydraSpecific() {
        flingySpeedPatches();
        //start of spell order code
        //default case of switch 004EC4FF
        unusedOrderPatches();
        phaseLinkPatches();//replaces CTFCop2 order
        lobotomyMinePatches();
        trapJumpWrappers();
        clarionWrappers();
        //lobotomyMineTargetingPatches();
        
        //add later: disable WarpIn functionality, make it useless
        
        archonHardcodePatches();
        /*
        
00431B26  |.  0F95C1        |SETNE CL
00431B29  |.  8BC1          |MOV EAX,ECX
00431B2B  |>  85C0          |TEST EAX,EAX
00431B2D  |.  0F84 70010000 |JE 00431CA3
        */

        /*
        
0043AFCD  |>  8BCB          |MOV ECX,EBX
0043AFCF  |.  E8 FCAA0300   |CALL 00475AD0
0043AFD4  |>  3C 82         |CMP AL,82
0043AFD6  |.  74 23         |JE SHORT 0043AFFB
        */

        /*
        
004418D9   .  E8 F2410300   CALL 00475AD0
        */

        /*
        
00441900   .  E8 CB410300   CALL 00475AD0
00441905   .  3C 82         CMP AL,82
        */

        /*
        
	Line 154324: 00467E37  |.  E8 94DC0000   CALL 00475AD0
	Line 175187: 00475AD0  /$  66:8B41 64    MOV AX,WORD PTR DS:[ECX+64]
	Line 175887: 004761F1  |>  E8 DAF8FFFF   CALL 00475AD0
	Line 176527: 0047684D  |.  E8 7EF2FFFF   CALL 00475AD0
	Line 178261: 00477AE8  |.  E8 E3DFFFFF   CALL 00475AD0
	Line 178537: 00477E36  |.  E8 95DCFFFF   CALL 00475AD0
	Line 179279: 00478640  |.  E8 8BD4FFFF   CALL 00475AD0
	Line 179802: 00478BC4  |.  E8 07CFFFFF   CALL 00475AD0
	Line 207274: 0048B0D1   .  E8 FAA9FEFF   CALL 00475AD0
	Line 321514: 004D818F  |.  E8 3CD9F9FF   |CALL 00475AD0
	Line 344811: 004E8076  |.  E8 55DAF8FF   CALL 00475AD0
        */
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