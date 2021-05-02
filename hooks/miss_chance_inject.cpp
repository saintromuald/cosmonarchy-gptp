#include "miss_chance.h"
#include <hook_tools.h>

namespace
{
	s32 __declspec(naked) canHitTargetCheck_Wrapper()
	{ // 004765B0
		static CUnit* a1;
		static CUnit* a2;
		static s32 result;
		__asm {
			PUSHAD
			MOV a1, EAX
			MOV a2, ECX
		}
		result = hooks::canHitTargetCheck(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	};
}

namespace hooks
{

	void injectMissChanceHooks()
	{
		// remove misschange when on height disadvantage
		jmpPatch(canHitTargetCheck_Wrapper, 0x004765B0, 1);

		// remove misschance for hitscan
		memoryPatch(0x0048C0C7, (u8)0x90);
		memoryPatch(0x0048C0C7 + 1, (u8)0xE9);

		// remove misschance for bullets
		memoryPatch(0x0048C1DB, (u8)0xEB);
	}
}