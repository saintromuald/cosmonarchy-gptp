#include "weapon_cooldown.h"
#include "../../hook_tools.h"

namespace
{
	void __declspec(naked) getModifiedWeaponCooldownWrapper()
	{
		static Unit* unit;
		static u8 weaponId;
		static u32 cooldown;
		__asm
		{
			MOV unit, ESI
			MOV weaponId, AL
			PUSHAD
		}
		cooldown = hooks::get_modified_weapon_cooldown(unit, weaponId);
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
	void injectWeaponCooldownHook()
	{
		jump_patch(getModifiedWeaponCooldownWrapper, 0x00475DC0, 2);
	}
}