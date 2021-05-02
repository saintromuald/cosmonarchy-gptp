#include "update_unit_state.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) updateUnitEnergyWrapper()
	{
		static CUnit* unit;
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

	void __declspec(naked) updateUnitStateWrapper()
	{
		static CUnit* unit;
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

	void __declspec(naked) incrementUnitScoresEx_Wrapper()
	{ // 00488D50
		static s32 count;
		static CUnit* unit;
		static s32 updateMorphMergeUnitScore;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV count, ECX
			MOV unit, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV updateMorphMergeUnitScore, EAX
			POP EAX
			PUSHAD
		}
		hooks::incrementUnitScoresEx(count, unit, updateMorphMergeUnitScore);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) updateUnits_Wrapper()
	{ // 004ED000
		__asm {
			PUSHAD
		}
		hooks::updateUnits();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) updateBurrowingCloakingUnits_Wrapper()
	{ // 00492EB0
		__asm {
			PUSHAD
		}
		hooks::updateBurrowingCloakingUnits();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) doCloak_Wrapper()
	{ // 0049B440
		static CUnit* unit;
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
}

namespace hooks
{
	void injectUpdateUnitState()
	{
		jmpPatch(updateUnitEnergy, 0x004EB4B0, 0);
		jmpPatch(updateUnitStateWrapper, 0x004EC290, 1);
		jmpPatch(incrementUnitScoresEx_Wrapper, 0x00488D50, 1);
		jmpPatch(updateUnits_Wrapper, 0x004ED000, 5);
		//jmpPatch(updateBurrowingCloakingUnits_Wrapper,	0x00492EB0, 3);
		//jmpPatch(doCloak_Wrapper,						0x0049B440, 5);
	}
}
