#include "creategame.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) generate_strength_table_wrapper() {
		__asm {
			PUSHAD
		}
		hooks::generate_strength_table();
		__asm {
			POPAD
			RETN
		}
	}
	
	void __declspec(naked) filter_unit_strength_wrapper() {
		static u32 unit_id;
		static u32 strength;
		static u32 result;
		__asm {
			MOV strength, ECX
			MOV unit_id, EAX
			PUSHAD
		}
		result = hooks::filter_unit_strength(unit_id, strength);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {
	void inject_creategame_hooks() {
		jump_patch(generate_strength_table_wrapper, 0x004313C0, 2);
		jump_patch(filter_unit_strength_wrapper,	0x00431150, 1);
	}
}