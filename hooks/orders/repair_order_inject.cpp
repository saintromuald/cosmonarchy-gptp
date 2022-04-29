#include "repair_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_Repair1Wrapper() // 004673D0
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_Repair1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) isOrderValid_Wrapper() // 00474D90
	{
		static s32 result;
		static char orderId;
		static Unit* unit;
		static s32 unused;
		static Unit* target;
		static s32 bErrorFlag;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV bErrorFlag, EAX
			POP EAX
			MOV target, ESI
			MOV unused, EDI
			MOV unit, EBX
			MOV orderId, AL
			PUSHAD
		}
		result = hooks::isOrderValid(orderId, unit, unused, target, bErrorFlag);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec() orderAllowed_Wrapper() // 0046DC20
	{
		static s32 result;
		static Unit* unit;
		static u16 orderId;
		static s32 player;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV orderId, BX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::orderAllowed(unit, orderId, player);
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
	void injectRepairOrderHook()
	{
		jmpPatch(orders_Repair1Wrapper,		0x004673D0, 1);
		jmpPatch(isOrderValid_Wrapper,		0x00474D90, 1);
//		jmpPatch(orderAllowed_Wrapper,		0x0046DC20, 3); // crashes
	}
}