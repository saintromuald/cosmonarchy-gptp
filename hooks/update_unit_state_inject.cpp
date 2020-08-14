#include "update_unit_state.h"
#include <hook_tools.h>

namespace {

void __declspec(naked) updateUnitEnergyWrapper() {

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


void __declspec(naked) updateUnitStateWrapper() {

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

void __declspec(naked) incrementUnitScoresEx_Wrapper() { // 0x00488D50
	static int count;
	static CUnit* unit;
	static int updateMorphMergeUnitScore;
	__asm {
		MOV count, ECX
		MOV unit, EDI
		PUSH updateMorphMergeUnitScore
		PUSHAD
	}
	hooks::incrementUnitScoresEx(count, unit, updateMorphMergeUnitScore);
	__asm {
		POPAD
		RETN
	}
}

} //unnamed namespace

namespace hooks {

void injectUpdateUnitState() {
	jmpPatch(updateUnitEnergy,				0x004EB4B0, 0);
	jmpPatch(updateUnitStateWrapper,		0x004EC290, 1);
	//jmpPatch(incrementUnitScoresEx_Wrapper, 0x00488D50, 1);
}

} //hooks
