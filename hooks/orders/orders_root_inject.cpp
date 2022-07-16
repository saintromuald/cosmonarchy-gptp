#include "orders_root.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) orders_id_cases_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_id_cases(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) secondary_orders_root_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::secondary_orders_root(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) main_orders_root_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::main_orders_root(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks {
	void inject_orders_root_hooks() {
		jump_patch(orders_id_cases_wrapper,			0x004EBED0,	5);
		jump_patch(secondary_orders_root_wrapper,	0x004EC170,	1);
		jump_patch(main_orders_root_wrapper,		0x004EC4D0,	3);
	}
}