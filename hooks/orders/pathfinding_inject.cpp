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
		result = hooks::move_to_target(target, unit);
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

	void __declspec(naked) unitHasPathToDest_Wrapper() // 0049CB60
	{
		static Unit* unit;
		static s32 x;
		static s32 y;
		static bool result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unitHasPathToDest(unit, x, y);
		__asm
		{
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) unitHasPathToUnit_Wrapper() // 0049CBB0
	{
		static Unit* pather;
		static Unit* target;
		static bool result;
		__asm
		{
			MOV pather, EAX
			MOV target, ECX
			PUSHAD
		}
		result = hooks::unitHasPathToUnit(pather, target);
		_asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}
}

namespace hooks
{
	void injectPathfindingHooks()
	{
//		jump_patch(moveToTarget_Wrapper,			0x004EB720, 0);
//		jump_patch(getActiveTileFlag_Wrapper,		0x00472F80, 1);
		jump_patch(unitHasPathToDest_Wrapper,		0x0049CB60, 1);
		jump_patch(unitHasPathToUnit_Wrapper,		0x0049CBB0, 0);
	}
}