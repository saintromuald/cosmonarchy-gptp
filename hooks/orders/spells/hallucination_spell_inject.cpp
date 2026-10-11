#include "hallucination_spell.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_hallucination1_wrapper() // 004F6C40
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_hallucination1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) create_hallucination_wrapper() // 004F6B90
	{
		static Unit* source;
		static s32 player;
		static Unit* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV source, ECX
			PUSHAD
		}
		result = hooks::create_hallucination(source, player);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_hallucination_spell_hooks()
	{
		jump_patch(orders_hallucination1_wrapper,	0x004F6C40, 0);
		jump_patch(create_hallucination_wrapper,	0x004F6B90, 2);
	}
}