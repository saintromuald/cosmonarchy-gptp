#include "unit_train.h"
#include <hook_tools.h>

namespace {

void __declspec(naked) secondaryOrd_TrainFighterWrapper() {

	static CUnit* unit;

	__asm {
		MOV unit, EAX
		PUSHAD
	}

	hooks::secondaryOrd_TrainFighter(unit);

	__asm {
		POPAD
		RETN
	}

}

;

void __declspec(naked) function_00468420Wrapper() {

	static CUnit* unit;

	__asm {
		MOV unit, EAX
		PUSHAD
	}

	hooks::function_00468420(unit);

	__asm {
		POPAD
		RETN
	}

}

}; //unnamed namespace

void __declspec(naked) queueSlotActiveWrapper() {
	static CUnit* unit;
	static u32 slot;
	static u32 result;
	__asm {
		MOV unit,ECX
		MOV slot,EDX
		PUSHAD
	}
	result = hooks::isQueueSlotActive(unit, slot);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

namespace hooks {

	void injectUnitTrainHooks() {
		jmpPatch(queueSlotActiveWrapper, 0x00401E70, 2);
		jmpPatch(secondaryOrd_TrainFighterWrapper,	0x00466790, 2);
		jmpPatch(function_00468420Wrapper,			0x00468420, 4);
	}

}; //hooks