//Injector source file for the Cast Order hook module.
#include "cast_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_Spell_Wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_Spell(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) spellOrder_Wrapper()
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
		result = hooks::spellOrder(defaultOrderString, caster, techId, range, energyCost);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) canTargetSpell_Wrapper() // 00492020
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
		result = hooks::canTargetSpell(techId, target, fowUnitId, caster, x, y, error);
		__asm
		{
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) issueOrderTargetingGround_Wrapper() // 00475260
	{
		static Unit* unit;
		static u8 id;
		static u16 coordX;
		static u16 coordY;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV id, AL
			MOV EAX, [EBP + 0x0C]
			MOV coordX, AX
			MOV EAX, [EBP + 0x10]
			MOV coordY, AX
			POP EAX
			MOV unit, ESI
			PUSHAD
		}
		hooks::issueOrderTargetingGround(unit, id, coordX, coordY);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}
}

namespace hooks
{
	void injectCastOrderHooks()
	{
		jmpPatch(orders_Spell_Wrapper,					0x00492850, 1);
		jmpPatch(spellOrder_Wrapper,					0x004926D0, 0);
		jmpPatch(canTargetSpell_Wrapper,				0x00492020, 1);
		jmpPatch(issueOrderTargetingGround_Wrapper,		0x00475260, 1);
	}
}
