#include "repair_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_repair_1_wrapper() // 004673D0
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_repair_1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) is_order_valid_wrapper() // 00474D90
	{
		static s32 result;
		static char order_id;
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
			MOV order_id, AL
			PUSHAD
		}
		result = hooks::is_order_valid(order_id, unit, unused, target, bErrorFlag);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec() order_allowed_wrapper() // 0046DC20
	{
		static s32 result;
		static Unit* unit;
		static u16 order_id;
		static s32 player;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV order_id, BX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::order_allowed(unit, order_id, player);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) show_right_click_error_if_needed_Wrapper() {
		static s32 result;
		static Unit* target;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV target, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::show_right_click_error_if_needed(target);
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
	void inject_repair_order_hooks()
	{
		jump_patch(orders_repair_1_wrapper,						0x004673D0, 1);
		jump_patch(is_order_valid_wrapper,						0x00474D90, 1);
//		jump_patch(order_allowed_wrapper,						0x0046DC20, 3); // crashes
		jump_patch(show_right_click_error_if_needed_Wrapper,	0x00455A00, 0);
	}
}