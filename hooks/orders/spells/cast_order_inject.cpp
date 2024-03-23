//Injector source file for the Cast Order hook module.
#include "cast_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_spell_wrapper() // 00492850
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_spell(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) spell_order_wrapper() // 004926D0
	{
		static s32 result;
		static s32 default_error_string;
		static Unit* caster;
		static u16 tech_id;
		static s32 range;
		static u16* energy_cost;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV tech_id, AX
			MOV EAX, [EBP + 0x0C]
			MOV range, EAX
			MOV EAX, [EBP + 0x10]
			MOV energy_cost, EAX
			POP EAX
			MOV caster, EBX
			MOV default_error_string, EAX
			PUSHAD
		}
		result = hooks::spell_order(default_error_string, caster, tech_id, range, energy_cost);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) can_target_spell_wrapper() // 00492020
	{
		static bool result;
		static u16 tech_id;
		static Unit* target;
		static s16 fow_unit_id;
		static Unit* caster;
		static s16 x;
		static s16 y;
		static u16* error;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV caster, EAX
			MOV EAX, [EBP + 0x0C]
			MOV x, AX
			MOV EAX, [EBP + 0x10]
			MOV y, AX
			MOV EAX, [EBP + 0x14]
			MOV error, EAX
			POP EAX
			MOV fow_unit_id, SI
			MOV target, EDI
			MOV tech_id, DX
			PUSHAD
		}
		result = hooks::can_target_spell(tech_id, target, fow_unit_id, caster, x, y, error);
		__asm {
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) issue_order_targeting_ground_wrapper() // 00475260
	{
		static Unit* unit;
		static u32 id;
		static u16 coord_x;
		static u16 coord_y;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV id, EAX
			MOV EAX, [EBP + 0x0C]
			MOV coord_x, AX
			MOV EAX, [EBP + 0x10]
			MOV coord_y, AX
			POP EAX
			MOV unit, ESI
			PUSHAD
		}
		hooks::issue_order_targeting_ground(unit, id, coord_x, coord_y);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) issue_tri_target_order_wrapper() // 0048CAF0
	{
		static char* unused;
		static u8 order_1;
		static u8 order_2;
		static u8 order_3;
		__asm
		{
			MOV unused, EDI
			MOV order_1, AL
			MOV order_2, DL
			MOV order_3, CL
			PUSHAD
		}
		hooks::issue_tri_target_order(order_1, order_2, order_3);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_defensive_matrix_wrapper() // 004550A0
	{
		static Unit* caster;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV caster, EAX
			POP EAX
			PUSHAD
		}
		hooks::orders_defensive_matrix(caster);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	// Patches \\

	const u32 sub_491DB0_91E6C = 0x00491E6C;
	void __declspec(naked) can_cast_ordered_spell_now_patch_91E64() {
	    __asm {
	        SAR		EAX, 8//y_halt
	        SAR		ECX, 8//x_halt
	        PUSH	0
	        PUSH	EBX//halt_distance + weapon_range
	        PUSH	ESI//unit
	        PUSH	EDI//order_target_pos.y
	        PUSH	EDX//order_target_pos.x
	        MOV		EDX, EAX
	        MOV		EBX, ECX
	        MOVZX	EAX, word ptr [ESI + 0x64]//unit->id
			SHL		EAX, 3
			ADD		EAX, units_dat::unit_bounds
			MOV		SI, [EAX + 6]//units_dat::unit_bounds[unit->id].bottom
			MOV		DI, DX
			SUB		DI, [EAX + 2]//units_dat::unit_bounds[unit->id].top
			SUB		CX, [EAX]//units_dat::unit_bounds[unit->id].left
			MOV		AX, [EAX + 4]//units_dat::unit_bounds[unit->id].right
	        ADD		SI, DX
	        INC		SI
	        ADD		AX, BX
	        INC		AX
	        POP		EBX//order_target_pos.x (x2)
	        MOVSX	EDX, CX
	        SUB		EDX, EBX
	        JNS		offset loc_430EE9
	        MOVSX	EAX, AX
	        SUB		EBX, EAX
	        MOV		EDX, EBX
	        JNS		offset loc_430EE9
	        XOR		EDX, EDX
	
	        loc_430EE9:
	        POP		ECX//order_target_pos.y
	        MOVSX	EAX, DI
	        SUB		EAX, ECX
	        JNS		offset loc_430EFF
	        MOVSX	EAX, SI
	        SUB		ECX, EAX
	        MOV		EAX, ECX
	        JNS		offset loc_430EFF
	        XOR		EAX, EAX
	
	        loc_430EFF:
	        MOV		EDI, EAX
	        POP		ESI//unit
	        POP		EBX//halt_distance + weapon_range
	        POP		ECX//x1 (0)
	        PUSH	EDI//y2 (order_target_pos.y)
	        PUSH	ECX//y1 (0)
	        JMP		sub_491DB0_91E6C
	    }
	}
}

namespace hooks
{
	void inject_cast_order_hooks()
	{
		jump_patch(orders_spell_wrapper,						0x00492850, 1);
		jump_patch(spell_order_wrapper,							0x004926D0, 0);
		jump_patch(can_target_spell_wrapper,					0x00492020, 1);
		jump_patch(issue_order_targeting_ground_wrapper,		0x00475260, 1);
		jump_patch(issue_tri_target_order_wrapper,				0x0048CAF0, 0);
		jump_patch(orders_defensive_matrix_wrapper,				0x004550A0, 0);
	}
}