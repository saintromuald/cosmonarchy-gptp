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
		seekRange = hooks::getSeekRangeHook(unit);
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
		maxWeaponRange = hooks::getMaxWeaponRangeHook(unit, weaponId, unit->orderTarget.unit);
		__asm
		{
			POPAD
			MOV EAX, maxWeaponRange;
			RETN
		}
	}

	void __declspec(naked) get_standard_attack_target_Wrapper() { // 00442DA0
		static s32 result;
		static Unit* target;
		static Unit* attacker;
		__asm {
			MOV target, ECX
			MOV attacker, EDX
			PUSHAD
		}
		result = hooks::get_standard_attack_target(target, attacker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

// Defined in SCBW/structures/Unit.cpp
extern const u32 Func_GetMaxWeaponRange;
extern const u32 Func_GetSeekRange;

namespace hooks
{
	void injectWeaponRangeHooks()
	{
		jmpPatch(getSeekRangeWrapper,					0x00476000, 3);
		jmpPatch(getMaxWeaponRangeWrapper,				0x00475870, 1);

		jmpPatch(get_standard_attack_target_Wrapper,	0x00442DA0, 2);
	}
}