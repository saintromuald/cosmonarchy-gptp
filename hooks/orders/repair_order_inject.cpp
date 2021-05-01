#include "repair_order.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) orders_Repair1Wrapper() {
		static CUnit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_Repair1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) isOrderValid_Wrapper() {
		static int32 result;
		static char orderId;
		static CUnit* unit;
		static int32 unused;
		static CUnit* target;
		static int32 bErrorFlag;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV bErrorFlag, EAX
			POP EAX
			MOV target, ESI
			MOV unused, EDI
			MOV unit, EBX
			MOVZX orderId, EAX
			PUSHAD
		}
		result = hooks::isOrderValid(orderId, unit, unused, target, bErrorFlag);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
} 

namespace hooks {

	void injectRepairOrderHook() {
		jmpPatch(orders_Repair1Wrapper,		0x004673D0, 1);
		jmpPatch(isOrderValid_Wrapper,		0x00474D90, 1);
	}

} //hooks