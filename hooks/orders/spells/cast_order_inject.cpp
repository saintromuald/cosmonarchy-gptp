//Injector source file for the Cast Order hook module.
#include "cast_order.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_SpellWrapper()
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
		static s32 energyCost;
		__asm
		{
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV techId, EAX
			MOV EAX, [EBP + 0xC]
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
			RETN 0xC
		}
	}
}

namespace hooks
{
	void injectCastOrderHooks()
	{
		jmpPatch(orders_SpellWrapper,	0x00492850, 1);
		jmpPatch(spellOrder_Wrapper,	0x004926D0, 0);
	}
}
