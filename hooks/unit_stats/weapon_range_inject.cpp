#include "weapon_range.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_seek_range_wrapper()
	{
		static Unit* unit;
		static u8 result;
		__asm
		{
			MOV unit, EDX
			PUSHAD
		}
		result = hooks::get_seek_range(unit, unit->order_target.unit);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) get_max_weapon_range_wrapper()
	{
		static Unit* unit;
		static u8 weapon_id;
		static u32 result;
		__asm
		{
			MOV unit, EAX
			MOV weapon_id, BL
			PUSHAD
		}
		result = hooks::get_max_weapon_range(unit, weapon_id, unit->order_target.unit);
		__asm
		{
			POPAD
			MOV EAX, result;
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
		jump_patch(get_seek_range_wrapper,			0x00476000, 3);
		jump_patch(get_max_weapon_range_wrapper,	0x00475870, 1);
	}
}