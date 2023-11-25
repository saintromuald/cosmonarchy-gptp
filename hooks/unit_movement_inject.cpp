#include "unit_movement.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) unit_movement_dormant_wrapper() // 0046A8D0
	{
		static Unit* unit;
		static s32 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_movement_dormant(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) move_unit_wrapper() // 004EB9F0
	{
		static Unit* unit;
		static s16 x;
		static s16 y;
		static s32 result;
		__asm
		{
			MOV unit, EDX
			MOV x, AX
			MOV y, CX
			PUSHAD
		}
		hooks::move_unit(unit, x, y, false);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) can_collide_with_wrapper() {
		static Unit* unit;
		static Unit* colliding_unit;
		static bool result;

		__asm {
			MOV unit, EDX
			MOV colliding_unit, ESI
			PUSHAD
		}

		result = hooks::can_collide_with(unit, colliding_unit);

		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_movement_start_path_wrapper() {
		static Unit* unit;
		static s32 result;

		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::unit_movement_start_path(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) move_towards_wrapper() {
		static Unit* target;
		static Unit* unit;
		static b32 result;

		__asm {
			MOV target, EAX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::move_towards(target, unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	// Patches \\

	const u32 playIdleIscript_978D8 = 0x004978D8;
	void __declspec(naked) play_idle_iscript_patch_978D3() {
		__asm {
			MOV		DL, [EBX + 0x20]
			AND		DL, 0xFD
			MOV		byte ptr[EBX + 0x20], DL	// unit->movement_flags &= ~MovementFlags::Accelerating
			MOV		ESI, [EAX + 0x1C]
			TEST	ESI, ESI
			JMP		playIdleIscript_978D8
		}
	}
}

namespace hooks
{
	void inject_unit_movement_hooks()
	{
		jump_patch(unit_movement_dormant_wrapper,		0x0046A8D0, 1);
	//	jump_patch(move_unit_wrapper,					0x004EB9F0, 1); // breaks sprite sorting
		jump_patch(can_collide_with_wrapper,			0x0042DEE0, 2);
	//	jump_patch(unit_movement_start_path_wrapper,	0x0046BD90, 1);
		jump_patch(move_towards_wrapper,				0x004EB900, 1);

	//	jump_patch(play_idle_iscript_patch_978D3,	0x004978D3); // DF - Fix animation bug with acceleration movement flag
	}
}