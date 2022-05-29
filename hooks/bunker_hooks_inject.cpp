#include "bunker_hooks.h"
#include "../hook_tools.h"

namespace
{
	const u32 Hook_UnitCanAttackInsideBunkerYes = 0x004790DC;
	void __declspec(naked) unitCanAttackInsideBunkerWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::unitAttackFromInsideBunkerHook(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) createBunkerAttackThingyWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::createBunkerAttackThingyHook(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectBunkerHooks()
	{
		jump_patch(unitCanAttackInsideBunkerWrapper, 0x004790A0, 2);
		jump_patch(createBunkerAttackThingyWrapper, 0x00477FD0, 3);
	}
}
