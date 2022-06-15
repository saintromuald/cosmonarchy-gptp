//Injector source file for the Apply Upgrade Flags hook module.
#include "apply_upgrade_flags.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) applyUpgradeFlagsToNewUnitWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		hooks::applyUpgradeFlagsToNewUnitHook(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) applyUpgradeFlagsToExistingUnitsWrapper()
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
		hooks::applyUpgradeFlagsToExistingUnitsHook(unit, upgradeId);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

}

namespace hooks
{
	void inject_apply_upgrade_flags_hooks()
	{
		jump_patch(applyUpgradeFlagsToNewUnitWrapper,		0x00454370, 2);
		jump_patch(applyUpgradeFlagsToExistingUnitsWrapper,	0x00454540, 2);
	}
}
