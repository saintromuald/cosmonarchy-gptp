#include "update_unit_state.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) updateUnitEnergyWrapper() // 004EB4B0
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::updateUnitEnergy(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) updateUnitStateWrapper() // 004EC290
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::updateUnitStateHook(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) updateUnits_Wrapper() // 004ED000
	{
		__asm {
			PUSHAD
		}
		hooks::updateUnits();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) updateBurrowingCloakingUnits_Wrapper() // 00492EB0
	{
		__asm {
			PUSHAD
		}
		hooks::updateBurrowingCloakingUnits();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) doCloak_Wrapper() // 0049B440
	{
		static Unit* unit;
		static s32 soundId;
		__asm {
			MOV soundId, EBX
			MOV unit, EAX
			PUSHAD
		}
		hooks::doCloak(unit, soundId);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) progressUnitFrame_Wrapper() // 004ECF70
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		hooks::progressUnitFrame(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectUpdateUnitState()
	{
		jump_patch(updateUnitEnergy,						0x004EB4B0, 0);
		jump_patch(updateUnitStateWrapper,				0x004EC290, 1);
		jump_patch(updateUnits_Wrapper,					0x004ED000, 5);
//		jump_patch(updateBurrowingCloakingUnits_Wrapper,	0x00492EB0, 3);
//		jump_patch(doCloak_Wrapper,						0x0049B440, 5);
		jump_patch(progressUnitFrame_Wrapper,				0x004ECF70, 3);
	}
}
