#include "feedback_spell.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_FeedbackWrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_feedback(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_feedback_spell()
	{
		jump_patch(orders_FeedbackWrapper,	0x004F6D40, 0);
	}
}