#include "new_requirements.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) create_unit_allowed_partial_wrapper() // 0046E1C0
	{
		bool runOriginal;
		static s32 result;
		static u16 unit_id;
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

	void __declspec(naked) unitreq_base_unit_type_patch_wrapper() {
		static u32 dat_req_offset;
		static u16 unit_id;

		__asm {
			PUSHAD
			MOV ECX, [EBP - 8]
			MOV EBP, ESP
			MOV unit_id, CX
			MOV dat_req_offset, EAX
		}

		unit_id = hooks::addon_datreq_fix(dat_req_offset, unit_id);

		__asm {
			POPAD
			MOV AX, unit_id
			RETN
		}
	}

	void __declspec(naked) do_place_building_checking_patch_wrapper() {
		static u32 dat_req_offset;
		static u16 unit_id;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV dat_req_offset, EAX
		}

		unit_id = hooks::addon_datreq_fix(dat_req_offset, *g_place_building_unit_id);

		__asm {
			POPAD
			MOV AX, unit_id
			RETN
		}
	}
}

namespace hooks {
	void inject_new_requirements_hooks() {
		jump_patch(create_unit_allowed_partial_wrapper,			0x0046E1C0, 1);
		jump_patch(can_set_rally_opcode_wrapper,				0x0046D8AF, 2);

	//	jump_patch(unitreq_base_unit_type_patch_wrapper,		0x0046E1A6, 3);
	//	jump_patch(do_place_building_checking_patch_wrapper,	0x0048E253, 3);
	}
}