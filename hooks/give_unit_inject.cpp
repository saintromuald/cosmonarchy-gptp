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
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) giveAI_Wrapper() // 00463040
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::giveAI(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectGiveUnitHook()
	{
		//jump_patch(giveUnit_Wrapper,		0x0049EFA0, 1);
		jump_patch(giveAI_Wrapper,		0x00463040,	1);
	}
}
