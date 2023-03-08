#include "recall_spell.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) spells_recall_action_on_valid_target_for_recall_wrapper()
	{
		static Unit* target;
		static Unit* caster;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV caster, EAX
			MOV target, EBX
			PUSHAD
		}
		hooks::spells_recall_action_on_valid_target_for_recall(target, caster);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) spells_recall_action_on_target_in_range_wrapper()
	{
		static Unit* target;
		static Unit* caster;
		__asm {
			MOV target, ECX
			MOV caster, EDX
			PUSHAD
		}
		hooks::spells_recall_action_on_target_in_range(target, caster);
		__asm {
			POPAD
			XOR EAX, EAX // EAX is cleared regardless of the result, keeping it for safety
			RETN
		}
	}

	void __declspec(naked) orders_recall_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_recall(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_recall_spell_hooks()
	{
		jump_patch(spells_recall_action_on_valid_target_for_recall_wrapper,	0x004942D0, 1);
		jump_patch(spells_recall_action_on_target_in_range_wrapper,			0x00494400, 3);
		jump_patch(orders_recall_wrapper,									0x00494470, 1);
	}
}