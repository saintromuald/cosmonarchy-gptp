#include "spider_mine.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) findBestTerranMineTarget_Wrapper()
	{
		static Unit* mine;
		static Unit* target;
		__asm {
			MOV mine, ESI
			PUSHAD
		}
		target = hooks::findBestTerranMineTarget(mine);
		__asm {
			POPAD
			MOV EAX, target
			RETN
		}
	}

	void __declspec(naked) orders_TerranMine_Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_TerranMine(unit);
		__asm {
			POPAD
			RETN
		}

	}

	void __declspec(naked) orders_PlaceMine_Wrapper() // 00464FD0
	{
		static Unit* a1;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		hooks::orders_PlaceMine(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) burrowGeneric_Wrapper() // 004E9A30
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::burrowGeneric(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectSpiderMineHooks()
	{
		jump_patch(findBestTerranMineTarget_Wrapper,	0x00441270, 1);
		jump_patch(orders_TerranMine_Wrapper,			0x00463DF0, 2);
//		jump_patch(orders_PlaceMine_Wrapper,			0x00464FD0, 1);
//		jump_patch(burrowGeneric_Wrapper,				0x004E9A30, 3);
	}
}
