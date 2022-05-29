#include "building_protoss.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_ProbeBuildWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_ProbeBuild(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_BuildSelf2Wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_BuildSelf2(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
}

namespace hooks
{
	void injectBuildingProtossHooks()
	{
		jump_patch(orders_ProbeBuildWrapper,	0x004E4D00, 1);
		jump_patch(orders_BuildSelf2Wrapper,	0x004E4F40, 0);
	}
}