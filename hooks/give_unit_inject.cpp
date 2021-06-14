//Injector source file for the Give Unit hook module.
#include "give_unit.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) giveUnit_Wrapper()
	{
		static Unit* unit;
		static u32 playerId;
		static u32 unkScore;

		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV ESP, EBP
			POP EAX
			POP EBP
			MOV unkScore, EAX
			MOV unit, ECX
			PUSHAD
		}
		hooks::giveUnit(unit, playerId, unkScore);
		__asm
		{
			POPAD
			RETN 0x08
		}
	}
}

namespace hooks
{
	void injectGiveUnitHook()
	{
		jmpPatch(giveUnit_Wrapper,		0x0049EFA0, 1);
	}
}
