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

	void __declspec(naked) getActiveTileFlag_Wrapper() // 00472F80
	{
		static b32 result;
		static s16 x;
		static s16 y;
		static s32 includedFlags;
		static s32 excludedFlags;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOVZX EAX, [EBP + 0x08]
			MOV x, AX
			MOV EAX, [EBP + 0x0C]
			MOV includedFlags, EAX
			MOV EAX, [EBP + 0x10]
			MOV excludedFlags, EAX
			POP EAX
			MOV y, AX
			PUSHAD
		}
		result = hooks::getActiveTileFlag(y, x, includedFlags, excludedFlags);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}
}

namespace hooks
{
	void injectPathfindingHooks()
	{
//		jmpPatch(moveToTarget_Wrapper,			0x004EB720, 0);
		jmpPatch(getActiveTileFlag_Wrapper,		0x00472F80, 1);
	}
}