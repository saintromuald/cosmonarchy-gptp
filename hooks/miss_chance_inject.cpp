#include "miss_chance.h"
#include <hook_tools.h>

namespace
{
	s32 __declspec(naked) can_hit_target_check_wrapper() // 004765B0
	{
		static Unit* target;
		static Unit* attacker;
		static s32 result;
		__asm {
			MOV target, EAX
			MOV attacker, ECX
			PUSHAD
		}
		result = hooks::can_hit_target_check(target, attacker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_miss_chance_hooks()
	{
		// remove miss change from height disadvantage
		jump_patch(can_hit_target_check_wrapper, 0x004765B0, 1);

		// remove miss chance for non-projectile attacks
		memory_patch(0x0048C0C7, (u8)0x90);
		memory_patch(0x0048C0C7 + 1, (u8)0xE9);

		// remove miss chance for projectile attacks
		memory_patch(0x0048C1DB, (u8)0xEB);
	}
}