#include "spellcasting.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) ai_spellcasting_wrapper()
	{
		static Unit* unit;
		static b32 is_under_attack;
		static b32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			MOV EAX, [EBP + 0x08]
			MOV is_under_attack, EAX
			PUSHAD
		}
		if (AI::ai_spellcaster_hook(unit, is_under_attack != 0))
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
		__asm
		{
			POPAD
			MOV EAX, result
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_spellcaster_ai_hooks()
	{
		jump_patch(ai_spellcasting_wrapper, 0x004A13C0, 2);
	}
}