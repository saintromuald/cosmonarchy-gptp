#include "trap_hooks.h"
#include "hook_tools.h"
#include "SCBW/api.h"

namespace {
	void _declspec(naked) ReturnFalse_Wrapper() {
		_asm {
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) unitIsFloorTrap_Wrapper() {
		static int a1;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		hooks::unitIsFloorTrap(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) disableUnit_Wrapper() {
		static CUnit* a1;
		static s16 result;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::disableUnit(a1);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) updateUnitStrengthAndApplyDefaultOrders_Wrapper() {
		static CUnit* a1;
		static u32 result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::updateUnitStrengthAndApplyDefaultOrders(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {
	void injectTrapHooks() {
		//jmpPatch(ReturnFalse_Wrapper, 0x00401430, 3);
		//jmpPatch(ReturnFalse_Wrapper, 0x0047B070, 3);
		jmpPatch(unitIsFloorTrap_Wrapper, 0x0047B070, 3);
		jmpPatch(disableUnit_Wrapper, 0x0047B4B0, 1);
//		jmpPatch(updateUnitStrengthAndApplyDefaultOrders_Wrapper, 0x0049FA40, 1);
	}
}