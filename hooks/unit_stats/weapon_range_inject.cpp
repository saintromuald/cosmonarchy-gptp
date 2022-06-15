#include "weapon_range.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) getSeekRangeWrapper()
	{
		static Unit* unit;
		static u8 seekRange;
		__asm
		{
			MOV unit, EDX
			PUSHAD
		}
		seekRange = hooks::getSeekRangeHook(unit, unit->order_target.unit);
		__asm
		{
			POPAD
			MOVZX EAX, seekRange
			RETN
		}
	}

	void __declspec(naked) getMaxWeaponRangeWrapper()
	{
		static Unit* unit;
		static u8 weaponId;
		static u32 maxWeaponRange;
		__asm
		{
			MOV unit, EAX
			MOV weaponId, BL
			PUSHAD
		}
		maxWeaponRange = hooks::getMaxWeaponRangeHook(unit, weaponId, unit->order_target.unit);
		__asm
		{
			POPAD
			MOV EAX, maxWeaponRange;
			RETN
		}
	}
}

// Defined in SCBW/structures/Unit.cpp
extern const u32 Func_GetMaxWeaponRange;
extern const u32 Func_GetSeekRange;

namespace hooks
{
	void inject_weapon_range_hooks()
	{
		jump_patch(getSeekRangeWrapper,			0x00476000, 3);
		jump_patch(getMaxWeaponRangeWrapper,	0x00475870, 1);
	}
}