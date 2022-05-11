#include "mindcontrol_spell.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_CastMindControlWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::ordersCastMindControl(unit);
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
	void injectMindControlSpellHook()
	{
		jmpPatch(orders_CastMindControlWrapper,		0x004F6950, 0);
	}
}