#include "pathfinding.h"
#include <hook_tools.h>
#include <SCBW/api.h>

namespace
{
	void __declspec(naked) moveToTarget_Wrapper() // 004EB720
	{
		static s32 result;
		static Unit* target;
		static Unit* unit;
		__asm
		{
			MOV target, EDI
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::moveToTarget(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectPathfindingHooks()
	{
		jmpPatch(moveToTarget_Wrapper, 0x004EB720, 0);
	}
}