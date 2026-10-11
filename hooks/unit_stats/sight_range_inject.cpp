#include "sight_range.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) get_sight_range_wrapper() { // 004E5B40
		static Unit* unit;
		static b32 is_for_spellcasting;
		static u32 sight_range;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDX
			MOV EAX, [EBP + 0x08]
			MOV is_for_spellcasting, EAX
			PUSHAD
		}
		sight_range = hooks::get_sight_range(unit, is_for_spellcasting != 0);
		__asm {
			POPAD
			MOV EAX, sight_range
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks {
	void inject_sight_range_hooks() {
		jump_patch(get_sight_range_wrapper,				0x004E5B40, 4);
	}
}