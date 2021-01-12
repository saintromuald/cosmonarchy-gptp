#include "target_related.h"
#include "hook_tools.h"
#include "SCBW/structures.h"

namespace {
	void __declspec(naked) genericRightClickUnitProc2_Wrapper() { // 00455E00
		static s32 result;
		static CUnit* unit;
		static s32 a2;
		static s16 a3;
		static CUnit* target;
		static s16 unitID;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a3, EAX
			MOV EAX, [EBP + 0xC]
			MOV target, EAX
			MOV EAX, [EBP + 0x10]
			MOV unitID, EAX
			POP EAX
			MOV a2, EDX
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::genericRightClickUnitProc2(unit, a2, a3, target, unitID);
		__asm {
			POPAD
			MOV result, EAX
			MOV ESP, EBP
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) CMDRECV_HoldPosition_Wrapper() { // 004C20C0
		static CUnit* result;
		static s32 a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::CMDRECV_HoldPosition(a1);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}
}

namespace hooks {

	void injectTargetHooks() {
		jmpPatch(genericRightClickUnitProc2_Wrapper, 0x00455E00, 1);
		jmpPatch(genericRightClickUnitProc2_Wrapper, 0x004C20C0, 0);
       
        nops(0x004EB176, 8);//disruption web
        nops(0x004EB316, 2);//disruption web

	}
	void injectTargetRelatedHooks() {
		injectTargetHooks();
	}
}