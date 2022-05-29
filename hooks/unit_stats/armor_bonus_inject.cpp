#include "armor_bonus.h"
#include <hook_tools.h>

extern const u32 Func_GetArmorBonus;	// Defined in Unit.cpp

namespace
{
	void __declspec(naked) getArmorBonusWrapper()
	{
		static Unit* unit;
		static u8 armor;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		armor = hooks::getArmorBonusHook(unit);
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
	void injectArmorBonusHook()
	{
		jump_patch(getArmorBonusWrapper, Func_GetArmorBonus, 1);
	}

}