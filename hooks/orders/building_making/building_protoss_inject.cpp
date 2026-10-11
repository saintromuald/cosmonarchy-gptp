#include "building_protoss.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) orders_scribe_build_wrapper() { // 004E4D00
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_scribe_build(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_build_self_2_wrapper() { // 004E4F40
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_build_self_2(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
}

namespace hooks {
	void inject_building_protoss_hooks() {
		jump_patch(orders_scribe_build_wrapper,	0x004E4D00, 1);
		jump_patch(orders_build_self_2_wrapper,	0x004E4F40, 0);
	}
}