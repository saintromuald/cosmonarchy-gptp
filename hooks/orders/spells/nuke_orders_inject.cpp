#include "nuke_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_nuke_track_wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EBX
			PUSHAD
		}
		hooks::orders_nuke_track(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) order_nuke_ground_wrapper()
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
		hooks::orders_nuke_ground(unit);
		__asm
		{
			POPAD
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_nuke_orders_hooks()
	{
		jump_patch(orders_nuke_track_wrapper,	0x00464D10, 2);
		jump_patch(order_nuke_ground_wrapper,	0x00479200, 0);
	}
}