#include "target_related.h"
#include "hook_tools.h"
#include "SCBW/structures.h"

/* TO WRAP
0043ADA0 = Ai_AskForHelp()
0043FFD0 = Ai_StimIfNeeded?()
00462EA0 = Ai_TryReturnHome()
00465F60 = Order_Interceptor
00477980 = Order_AttackUnit_Subunit()
00478540 = Order_AttackUnit_Main()
00478B40 = AttackAtPoint()
00491C20 = Order_CloakingNearby()
*/

namespace hooks {
	/*
    bool isUnitInWeaponRange(CUnit* a1, CUnit* a2) {
        
        __asm {
            PUSHAD
            push    ebp
            mov     ebp, esp
            push    ebx
            mov     ebx, [ebp + a2]
            push    edi
            mov     edi, eax
            test    edi, edi
            jnz     short loc_476890
            mov     edi, [ebx + 5Ch]
            test    edi, edi
            jnz     short loc_476890
            pop     edi
            mov     eax, 1
            pop     ebx
            pop     ebp
            retn    4

            loc_476890 :
            push    esi
            mov     eax, edi
            mov     esi, ebx
            call    isTargetVisible
            test    eax, eax
            jnz     short loc_4768A7

            loc_47689E:
            pop     esi
            pop     edi
            xor eax, eax
            pop     ebx
            pop     ebp
            retn    4

            loc_4768A7:
            mov     esi, [ebx + 70h]
            test    esi, esi
            mov     cl, 10h
            jz      short loc_4768BD
            movzx   eax, word ptr[esi + 64h]
            test    byte ptr unitsdat_SpecialAbilityFlags[eax * 4], cl
            jnz     short loc_4768BF

            loc_4768BD:
            mov     esi, ebx

            loc_4768BF :
            test    byte ptr[edi + 0DCh], 4
            jz      short loc_4768D4
            movzx   ecx, word ptr[esi + 64h]
            mov     bl, unitsdat_AirWeapon[ecx]
            jmp     short loc_4768F3

            loc_4768D4:
            mov     ax, [esi + 64h]
            cmp     ax, 67h
            jnz     short loc_4768EA
            test[esi + 0DCh], cl
            jnz     short loc_4768EA
            mov     bl, 82h
            jmp     short loc_4768F3


            loc_4768EA:
            movzx   edx, ax
            mov     bl, unitsdat_GroundWeapon[edx]

            loc_4768F3 :
            cmp     bl, 82h
            jnb     short loc_47689E
            movzx   eax, bl
            mov     eax, weaponsdat_minRange[eax * 4]
            test    eax, eax
            jz      short loc_476913
            push    edi
            push    eax
            mov     ecx, esi
            call    isTargetWithinMinRange
            test    eax, eax
            jnz     short loc_47689E

            loc_476913:
            mov     eax, [ebp + a2]
            call    getUnitMaxWpnRange
            mov     ecx, eax
            mov     eax, edi
            mov     edi, esi
            call    isTargetWithinMinMovementRange
            pop     esi
            pop     edi
            pop     ebx
            pop     ebp
            retn    4
        }
    }*/

/*
	void injectTargetHooks() {
		// reference hooks
		//jmpPatch(Ai_ProgressSpendingQueue_JmpWrapper, 0x00434C31);
		//jmpPatch(Ai_BuildSupplies_Wrapper, 0x00433730);
		//jmpPatch(Ai_GetDefaultWorkerFromRace, 0x00432130);
	}
	void injectTargetRelatedHooks() {
		injectTargetHooks();
	}*/
}

/*

//reference wrapper

	const u32 jmp_34c53 = 0x00434C53;
	const u32 jmp_4C80_ret = 0x00434C80;
	void __declspec(naked) Ai_ProgressSpendingQueue_JmpWrapper() {
		static u32 unit_id;
		static u32 result;
		__asm {
			MOV unit_id, EAX
			PUSHAD
		}
		result = get_order_from_unit_id(unit_id);
		if(result==0){//default case
			__asm {
				POPAD
				JMP jmp_4C80_ret
			}
		}
		else {
			__asm {
				POPAD
				MOV EAX,result
				JMP jmp_34c53
			}
		}
	}

*/