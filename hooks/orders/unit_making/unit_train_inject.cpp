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

	void __declspec(naked) queueSlotActiveWrapper() {
		static CUnit* unit;
		static uint32 slot;
		static uint32 result;
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

	void __declspec(naked) getMaxHangarSpace_Wrapper() {
		static CUnit* a1;
		static char result;
		__asm {
			MOV a1, ECX
			PUSHAD
		}
		result = hooks::getMaxHangerSpace(a1);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

}

namespace hooks {

	void injectUnitTrainHooks() {
		jmpPatch(queueSlotActiveWrapper, 0x00401E70, 2);
		jmpPatch(secondaryOrd_TrainFighterWrapper,	0x00466790, 2);
		jmpPatch(function_00468420Wrapper,			0x00468420, 4);
		jmpPatch(getMaxHangarSpace_Wrapper, 		0x004653D0, 3);
	}

}; //hooks