#include "give_unit.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) give_unit_wrapper()
	{
		static Unit* unit;
		static u32 player;
		static u32 count_as_completed;
		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV ESP, EBP
			POP EAX
			POP EBP
			MOV count_as_completed, EAX
			MOV unit, ECX
			PUSHAD
		}
		hooks::give_unit(unit, player, count_as_completed);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) give_ai_wrapper() // 00463040
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::give_ai(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_give_unit_hooks()
	{
	//	jump_patch(give_unit_wrapper,	0x0049EFA0, 1);
		jump_patch(give_ai_wrapper,		0x00463040,	1);
	}
}
