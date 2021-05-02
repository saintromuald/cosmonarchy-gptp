#include "spider_mine.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) findBestTerranMineTarget_Wrapper()
	{
		static CUnit* mine;
		static CUnit* target;
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
		static CUnit* unit;
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

	void __declspec(naked) orders_PlaceMine_Wrapper()
	{
		static CUnit* a1;
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

	void __declspec(naked) burrowGeneric_Wrapper()
	{ // 004E9A30
		static CUnit* unit;
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
		jmpPatch(findBestTerranMineTarget_Wrapper, 0x00441270, 1);
		jmpPatch(orders_TerranMine_Wrapper, 0x00463DF0, 2);
		//		jmpPatch(orders_PlaceMine_Wrapper,			0x00464FD0, 1);
		//		jmpPatch(burrowGeneric_Wrapper,				0x004E9A30, 3);
	}
}
