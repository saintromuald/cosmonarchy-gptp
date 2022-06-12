#include "unit_train.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) secondaryOrd_TrainFighterWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::secondaryOrd_TrainFighter(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) function_00468420Wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::orderTrain(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) queueSlotActiveWrapper()
	{
		static Unit* unit;
		static u32 slot;
		static u32 result;
		__asm
		{
			MOV unit, ECX
			MOV slot, EDX
			PUSHAD
		}
		result = hooks::is_queue_slot_active(unit, slot);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getMaxHangarSpace_Wrapper()
	{
		static Unit* a1;
		static char result;
		__asm
		{
			MOV a1, ECX
			PUSHAD
		}
		result = hooks::getMaxHangerSpace(a1);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	const u32 loc_success = 0x0046DA3E;
	const u32 loc_fail_A = 0x0046DAC7;
	void __declspec(naked) isLiftedOff_Wrapper() // 0046D832
	{
		static Unit* unit;
		_asm
		{
			MOV unit, ESI
			PUSHAD
		}
		if (unit->status & UnitStatus::InAir)
		{
			_asm
			{
				POPAD
				JMP loc_fail_A
			}
		}
		_asm
		{
			POPAD
			INC edi
			JMP loc_success
		}
	}

	const u32 loc_fail_B = 0x0046DADB;
	void __declspec(naked) isNotLiftedOff_Wrapper() // 0046D845
	{
		static Unit* unit;
		_asm
		{
			MOV unit, ESI
			PUSHAD
		}
		if (!(unit->status & UnitStatus::InAir))
		{
			_asm
			{
				POPAD
				JMP loc_fail_B
			}
		}
		_asm
		{
			POPAD
			INC edi
			JMP loc_success
		}
	}
}

namespace hooks
{
	void injectUnitTrainHooks() {
		jump_patch(queueSlotActiveWrapper,				0x00401E70, 2);
		jump_patch(secondaryOrd_TrainFighterWrapper,	0x00466790, 2);
		jump_patch(function_00468420Wrapper,			0x00468420, 4);
		jump_patch(getMaxHangarSpace_Wrapper,			0x004653D0, 3);
		jump_patch(ButtonIsTraining,					0x00428530, 0);
		jump_patch(isLiftedOff_Wrapper,					0x00046D832, 3);
		jump_patch(isNotLiftedOff_Wrapper,				0x0046D845, 2);
	}
}