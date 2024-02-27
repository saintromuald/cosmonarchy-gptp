#include "die_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_die_wrapper() // 00479480
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_die(unit);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_die_order_hooks()
	{
		jump_patch(orders_die_wrapper,	0x00479480, 2);
	}
}