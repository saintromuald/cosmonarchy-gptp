#include "sight_range.h"
#include <hook_tools.h>

extern const u32 Func_GetSightRange; // Defined in Unit.cpp
namespace
{
	void __declspec(naked) getSightRangeWrapper()
	{
		static Unit* unit;
		static b32 isForSpellCasting;
		static u32 sightRange;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDX
			MOV EAX, [EBP + 0x08]
			MOV isForSpellCasting, EAX
			PUSHAD
		}
		sightRange = hooks::getSightRangeHook(unit, isForSpellCasting != 0);
		__asm {
			POPAD
			MOV EAX, sightRange
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_sight_range_hooks()
	{
		jump_patch(getSightRangeWrapper, Func_GetSightRange, 4);
	}
}