#include "target_related.h"
#include "hook_tools.h"

/* TO WRAP
0043ADA0 = Ai_AskForHelp()
0043FFD0 = Ai_StimIfNeeded?()
00462EA0 = Ai_TryReturnHome()
00465F60 = Order_Interceptor
00476430 = IsOutOfRange() // WILL HOOK
00476640 = IsReadyToAttack()
00476870 = IsInAttackRange()
00477980 = Order_AttackUnit_Subunit()
00478540 = Order_AttackUnit_Main()
00478B40 = AttackAtPoint()
orders_WatchTarget 0047BAB0
orders_StayInRange 0047C4F0
00491C20 = Order_CloakingNearby()
00491DB0 = CanCastOrderedSpellNow()
*/

/*
orders_StrafeUnit proc near             ; CODE XREF: ordersEntries+5AB?j

target          = dword ptr -4

                push    ebp
                mov     ebp, esp
                push    ecx
                push    esi
                mov     esi, eax        ; unit
                mov     eax, [esi+0C0h]
                test    eax, eax        ; Logical Compare
                jz      short loc_465F91 ; Jump if Zero (ZF=1)
                movzx   eax, guwUnitShields+92h ; 26
                mov     ecx, [esi+60h]
                shl     eax, 8          ; Shift Logical Left
                sar     eax, 2          ; Shift Arithmetic Right
                cmp     ecx, eax        ; Compare Two Operands
                jge     short loc_465F91 ; Jump if Greater or Equal (SF=OF)
                mov     cl, 45h         ; orderID
                call    orderComputer_cl ; 175
                pop     esi
                mov     esp, ebp
                pop     ebp
                retn                    ; Return Near from Procedure
*/

namespace hooks {



	void injectTargetHooks() {
		// reference hooks
		//jmpPatch(Ai_ProgressSpendingQueue_JmpWrapper, 0x00434C31);
		//jmpPatch(Ai_BuildSupplies_Wrapper, 0x00433730);
		//jmpPatch(Ai_GetDefaultWorkerFromRace, 0x00432130);
	}
	void injectTargetRelatedHooks() {
		injectTargetHooks();
	}
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