#include "unit_movement.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) unit_movement_dormant_wrapper() { // 0046A8D0
		static Unit* unit;
		static b32 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_movement_dormant(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) move_unit_wrapper() { // 004EB9F0
		static Unit* unit;
		static s16 x;
		static s16 y;
		static s32 result;
		__asm {
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

	void __declspec(naked) can_collide_with_wrapper() { // 0042DEE0
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

	void __declspec(naked) unit_movement_start_path_wrapper() { // 0046BD90
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

	void __declspec(naked) move_towards_wrapper() { // 004EB900
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

	void __declspec(naked) unit_movement_initialize_wrapper() { // 0046C320
		static Unit* unit;
		static s32 result;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::unit_movement_initialize(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) refresh_unit_after_move_wrapper() { // 00494160
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::refresh_unit_after_move(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) decollide_unit_wrapper() { // 004F2DE0
		static Unit* unit;
		static b32 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::decollide_unit(unit);
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

	void __declspec(naked) can_move_unit_patch() {
		static const u32 can_move_unit_9D3E8 = 0x0049D3E8;
		static const u32 can_move_unit_9D52F = 0x0049D52F;
		__asm {
			SUB ESP, 0x14
			PUSH EBX
			PUSH ESI
			MOV ESI, [EBP + 0x8]
			CMP[ESI + 0x64], 101//UnitId::Special_MapRevealer
			JE offset _ret
			JMP can_move_unit_9D3E8

			_ret :
			PUSH EDI
				JMP can_move_unit_9D52F
		}
	}
}

namespace hooks {
	void inject_unit_movement_hooks() {
		jump_patch(unit_movement_dormant_wrapper, 0x0046A8D0, 1);
	//	jump_patch(move_unit_wrapper,							0x004EB9F0, 1); // breaks sprite sorting
		jump_patch(can_collide_with_wrapper,		0x0042DEE0, 2);
	//	jump_patch(unit_movement_start_path_wrapper,			0x0046BD90, 1);
		jump_patch(move_towards_wrapper,			0x004EB900, 1);
	//	jump_patch(unit_movement_initialize_wrapper,			0x0046C320, 4); // Not tested yet
	//	jump_patch(refresh_unit_after_move_wrapper,				0x00494160,	2); // Causes common freeze
	//	jump_patch(decollide_unit_wrapper,						0x004F2DE0, 1); // Not tested yet

	//	jump_patch(play_idle_iscript_patch_978D3,				0x004978D3); // DF - Fix animation bug with acceleration movement flag
		jump_patch(can_move_unit_patch,			0x0049D3E3); // DF - Fix map revealers crashing on map start if colliding with air units
	}
}