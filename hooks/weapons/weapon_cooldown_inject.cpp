#include "weapon_cooldown.h"
#include "../../hook_tools.h"

namespace
{
	void __declspec(naked) get_modified_weapon_cooldown_wrapper()
	{
		static Unit* unit;
		static u8 weapon_id;
		static u32 cooldown;
		__asm
		{
			MOV unit, ESI
			MOV weapon_id, AL
			PUSHAD
		}
		cooldown = hooks::get_modified_weapon_cooldown(unit, weapon_id);
		__asm
		{
			POPAD
			MOV EAX, cooldown
			RETN
		}
	}
}

namespace hooks
{
	void inject_weapon_cooldown_hooks()
	{
		jump_patch(get_modified_weapon_cooldown_wrapper, 0x00475DC0, 2);
	}
}