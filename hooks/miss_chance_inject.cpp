#include "miss_chance.h"
#include <hook_tools.h>

namespace
{
	s32 __declspec(naked) canHitTargetCheck_Wrapper() // 004765B0
	{
		static Unit* a1;
		static Unit* a2;
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
	}
}

namespace hooks
{

	void injectMissChanceHooks()
	{
		// remove miss change from height disadvantage
		jump_patch(canHitTargetCheck_Wrapper, 0x004765B0, 1);

		// remove miss chance for non-projectile attacks
		memory_patch(0x0048C0C7, (u8)0x90);
		memory_patch(0x0048C0C7 + 1, (u8)0xE9);

		// remove miss chance for projectile attacks
		memory_patch(0x0048C1DB, (u8)0xEB);
	}
}