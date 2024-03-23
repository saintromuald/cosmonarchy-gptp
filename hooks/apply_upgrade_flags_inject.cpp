#include "apply_upgrade_flags.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) apply_upgrade_flags_to_new_unit_wrapper() // 00454370
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		hooks::apply_upgrade_flags_to_new_unit(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) apply_upgrade_flags_to_existing_units_wrapper() // 00454540
	{
		static Unit* unit;
		static u8 upgradeId;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV upgradeId, AL
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::apply_upgrade_flags_to_existing_units(unit, upgradeId);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

}

namespace hooks
{
	void inject_apply_upgrade_flags_hooks()
	{
		jump_patch(apply_upgrade_flags_to_new_unit_wrapper,			0x00454370, 2);
		jump_patch(apply_upgrade_flags_to_existing_units_wrapper,	0x00454540, 2);
	}
}