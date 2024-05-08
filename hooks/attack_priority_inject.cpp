#include "attack_priority.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_attack_priority_wrapper() // 00442160
	{
		static Unit* target, * attacker;
		static u32 result;
		__asm {
			MOV target, EDI
			MOV attacker, EBX
			PUSHAD
		}
		result = hooks::get_attack_priority(target, attacker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) find_best_attack_target_wrapper() // 00443080
	{
		static Unit* unit;
		static Unit* target;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		target = hooks::find_best_attack_target(unit);
		__asm {
			POPAD
			MOV EAX, target
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) find_random_attack_target_wrapper() // 00442FC0
	{
		static Unit* unit;
		static Unit* target;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, ESI
			PUSHAD
		}
		target = hooks::find_random_attack_target(unit);
		__asm {
			POPAD
			MOV EAX, target
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
}

namespace hooks
{
	void inject_attack_priority_hooks()
	{
		jump_patch(get_attack_priority_wrapper,			0x00442160, 3);
		jump_patch(find_best_attack_target_wrapper,		0x00443080, 1);
		jump_patch(find_random_attack_target_wrapper,	0x00442FC0, 1);
	}
}
