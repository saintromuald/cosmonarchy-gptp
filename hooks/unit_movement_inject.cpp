#include "unit_movement.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) unitMovementDormant_Wrapper() // 0046A8D0
	{
		static Unit* unit;
		static s32 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unitMovementDormant(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) MoveUnit_Wrapper() // 004EB9F0
	{
		static Unit* unit;
		static s16 x;
		static s16 y;
		static s32 result;
		__asm
		{
			MOV unit, EDX
			MOV x, AX
			MOV y, CX
			PUSHAD
		}
		hooks::MoveUnit(unit, x, y, false);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectUnitMovementHooks()
	{
		jump_patch(unitMovementDormant_Wrapper,	0x0046A8D0, 1);
	//	jump_patch(MoveUnit_Wrapper,			0x004EB9F0, 1); // breaks sprite sorting
	}
}