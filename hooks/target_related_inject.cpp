#include "target_related.h"
#include "hook_tools.h"
#include "SCBW/structures.h"

namespace {
	void __declspec(naked) genericRightClickUnitProc2_Wrapper() { // 00455E00
		static int32 result;
		static CUnit* unit;
		static int32 a2;
		static int16 a3;
		static CUnit* target;
		static int16 unitID;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a3, AX
			MOV EAX, [EBP + 0xC]
			MOV target, EAX
			MOV EAX, [EBP + 0x10]
			MOV unitID, AX
			POP EAX
			MOV a2, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::genericRightClickUnitProc2(unit, a2, a3, target, unitID);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) CMDRECV_HoldPosition_Wrapper() { // 004C20C0
		static CUnit* result;
		static int32 a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::CMDRECV_HoldPosition(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unitCanSeeCloakedTarget_Wrapper() { // 00401D60
		static CUnit* target;
		static CUnit* unit;
		static uint32 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV target, EAX
			PUSHAD
		}
		result = hooks::unitCanSeeCloakedTarget(target, unit);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) isTargetVisible_Wrapper() { // 004E5E30
		static uint32 result;
		static CUnit* target;
		static CUnit* unit;
		__asm {
			MOV target, EAX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::isTargetVisible(target, unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) isUnitVisible_Wrapper() { // 004E5DB0
		static uint32 result;
		static CUnit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::isUnitVisible(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) checkUnitVisibility_Wrapper() { // 00443390
		static CUnit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::checkUnitVisibility(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) isUnitCloaking_Wrapper() { // 004916E0
		static int32 result;
		static CUnit* unit;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::isUnitCloaking(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {

	void injectTargetHooks() {
//		jmpPatch(genericRightClickUnitProc2_Wrapper, 0x00455E00, 1);
//		jmpPatch(genericRightClickUnitProc2_Wrapper, 0x004C20C0, 0);
		nops(0x004EB176, 8); // disruption web
		nops(0x004EB316, 2); // disruption web
		jmpPatch(unitCanSeeCloakedTarget_Wrapper, 0x00401D60, 4);
//		jmpPatch(isTargetVisible_Wrapper, 0x004E5E30, 4);
//		jmpPatch(isUnitVisible_Wrapper, 0x004E5DB0, 1);
		jmpPatch(checkUnitVisibility_Wrapper, 0x00443390, 3);
		jmpPatch(isUnitCloaking_Wrapper, 0x004916E0, 0);
	}
	void injectTargetRelatedHooks() {
		injectTargetHooks();
	}
}