#include "merge_units.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) templarMergePartnerWrapper() // 004BFA80
	{
		static Unit* unit;				//2nd push
		static Unit** units_list;		//1st push
		static u32 units_list_length;	//eax
		static Unit* nearestPartner;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV units_list_length, EAX
			MOV EAX, [EBP + 0x0C]
			MOV units_list, EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		nearestPartner = hooks::templarMergePartner(unit, units_list, units_list_length);
		__asm {
			POPAD
			MOV EAX, nearestPartner
			MOV ESP, EBP
			POP EBP
			RETN 0x0008
		}
	}

	void __declspec(naked) orders_CompletingArchonSummonWrapper() // 00493B10
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV unit, ESI
			PUSHAD
		}
		hooks::orders_CompletingArchonSummon(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) orders_WarpingDarkArchonWrapper() // 00494690
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_WarpingDarkArchon(unit);
		__asm {
			POPAD
			RET
		}
	}

	void __declspec(naked) orders_WarpingArchonWrapper() // 004948B0
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_WarpingArchon(unit);
		__asm {
			POPAD
			RET
		}
	}
}

namespace hooks
{
	void injectMergeUnitsHooks()
	{
		jump_patch(templarMergePartnerWrapper,				0x004BFA80, 1);
		jump_patch(orders_CompletingArchonSummonWrapper,	0x00493B10, 1);
		jump_patch(orders_WarpingDarkArchonWrapper,			0x00494690, 2);
		jump_patch(orders_WarpingArchonWrapper,				0x004948B0, 2);
	}
}
