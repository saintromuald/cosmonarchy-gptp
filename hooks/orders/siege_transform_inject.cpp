#include "siege_transform.h"
#include <hook_tools.h>

void __declspec(naked) orders_tank_mode_wrapper() // 00464AE0
{
	static Unit* unit;
	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unit, EAX
	}
	hooks::orders_tank_mode(unit);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) orders_siege_mode_wrapper() // 00464BD0
{
	static Unit* unit;
	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unit, EDI
	}
	hooks::orders_siege_mode(unit);
	__asm {
		POPAD
		RETN
	}
}

namespace hooks
{
	void inject_siege_transform_hooks()
	{
		jump_patch(orders_tank_mode_wrapper,		0x00464AE0, 1);
		jump_patch(orders_siege_mode_wrapper,		0x00464BD0, 1);
		jump_patch(siege_tank_self_destruct_proc,	0x00463F90, 1);
	}
}