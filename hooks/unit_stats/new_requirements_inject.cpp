#include "new_requirements.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) create_unit_allowed_partial_wrapper() // 0046E1C0
	{
		bool runOriginal;
		static s32 result;
		static s16 unit_id;
		static Unit* unit;
		static s32 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV unit_id, AX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::create_unit_allowed_partial(unit_id, unit, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) can_set_rally_opcode_wrapper() {
		static const u32 jmp_failed  = 0x0046DB03;
		static const u32 jmp_success = 0x0046D8BC;
		static Unit* unit;
		static bool result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::can_set_rally_opcode(unit);
		if (result) {
			__asm {
				POPAD
				JMP jmp_success
			}
		}
		__asm {
			POPAD
			JMP jmp_failed
		}
	}
}

namespace hooks {
	void injectNewRequirementsHooks() {
		jump_patch(create_unit_allowed_partial_wrapper, 0x0046E1C0, 1);
		jump_patch(can_set_rally_opcode_wrapper, 0x0046D8AF, 2);
	}
}