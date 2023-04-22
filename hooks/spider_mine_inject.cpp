#include "spider_mine.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) find_best_mine_target_wrapper()
	{
		static Unit* mine;
		static Unit* target;
		__asm {
			MOV mine, ESI
			PUSHAD
		}
		target = hooks::find_best_mine_target(mine);
		__asm {
			POPAD
			MOV EAX, target
			RETN
		}
	}

	void __declspec(naked) orders_terran_mine_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_terran_mine(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_place_mine_wrapper() // 00464FD0
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_place_mine(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) burrow_generic_wrapper() // 004E9A30
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::burrow_generic(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_spider_mine_hooks()
	{
		jump_patch(find_best_mine_target_wrapper,		0x00441270, 1);
		jump_patch(orders_terran_mine_wrapper,			0x00463DF0, 2);
	//	jump_patch(orders_place_mine_wrapper,			0x00464FD0, 1);
	//	jump_patch(burrow_generic_wrapper,				0x004E9A30, 3);
	}
}