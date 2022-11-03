#include "armor_bonus.h"
#include <hook_tools.h>

extern const u32 Func_GetArmorBonus; // Defined in fuck you visual studio.cpp
namespace
{
	void __declspec(naked) get_armor_bonus_wrapper()
	{
		static Unit* unit;
		static u8 armor;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		armor = hooks::get_armor_bonus(unit);
		__asm
		{
			POPAD
			MOVZX EAX, armor
			RETN
		}
	}
}

namespace hooks
{
	void inject_armor_bonus_hooks()
	{
		jump_patch(get_armor_bonus_wrapper, Func_GetArmorBonus, 1);
	}
}