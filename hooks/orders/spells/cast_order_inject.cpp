//Injector source file for the Cast Order hook module.
#include "cast_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_spell_wrapper()
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

	void __declspec(naked) spell_order_wrapper()
	{
		static s32 result;
		static s32 defaultOrderString;
		static Unit* caster;
		static u16 techId;
		static s32 range;
		static u16* energyCost;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV techId, AX
			MOV EAX, [EBP + 0x0C]
			MOV range, EAX
			MOV EAX, [EBP + 0x10]
			MOV energyCost, EAX
			POP EAX
			MOV caster, EBX
			MOV defaultOrderString, EAX
			PUSHAD
		}
		result = hooks::spell_order(defaultOrderString, caster, techId, range, energyCost);
		__asm
		{
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
		static u16 techId;
		static Unit* target;
		static s16 fowUnitId;
		static Unit* caster;
		static s16 x;
		static s16 y;
		static u16* error;
		__asm
		{
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
			MOV fowUnitId, SI
			MOV target, EDI
			MOV techId, DX
			PUSHAD
		}
		result = hooks::can_target_spell(techId, target, fowUnitId, caster, x, y, error);
		__asm
		{
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
		static u16 coordX;
		static u16 coordY;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV id, EAX
			MOV EAX, [EBP + 0x0C]
			MOV coordX, AX
			MOV EAX, [EBP + 0x10]
			MOV coordY, AX
			POP EAX
			MOV unit, ESI
			PUSHAD
		}
		hooks::issue_order_targeting_ground(unit, id, coordX, coordY);
		__asm
		{
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
	}
}
