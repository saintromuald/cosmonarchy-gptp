#include "unit_train.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) secondary_order_train_fighter_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::secondary_order_train_fighter(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) order_train_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::order_train(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) queue_slot_active_wrapper()
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

	void __declspec(naked) get_max_hangar_space_wrapper()
	{
		static Unit* a1;
		static char result;
		__asm
		{
			MOV a1, ECX
			PUSHAD
		}
		result = hooks::get_max_hangar_space(a1);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) update_unit_stats_and_finish_building_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::update_unit_stats_and_finish_building(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 loc_success = 0x0046DA3E;
	const u32 loc_fail_A = 0x0046DAC7;
	void __declspec(naked) is_lifted_off_wrapper() // 0046D832
	{
		static Unit* unit;
		_asm
		{
			MOV unit, ESI
			PUSHAD
		}
		if (unit->status & UnitStatus::InAir) {
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
	void __declspec(naked) is_not_lifted_off_wrapper() // 0046D845
	{
		static Unit* unit;
		_asm
		{
			MOV unit, ESI
			PUSHAD
		}
		if (!(unit->status & UnitStatus::InAir)) {
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
	void inject_unit_train_hooks() {
		jump_patch(queue_slot_active_wrapper,						0x00401E70, 2);
		jump_patch(secondary_order_train_fighter_wrapper,			0x00466790, 2);
		jump_patch(order_train_wrapper,								0x00468420, 4);
		jump_patch(get_max_hangar_space_wrapper,					0x004653D0, 3);
		jump_patch(button_is_training,								0x00428530, 0);
		jump_patch(update_unit_stats_and_finish_building_wrapper,	0x004A01F0, 1);

		jump_patch(is_lifted_off_wrapper,							0x0046D832, 3);
		jump_patch(is_not_lifted_off_wrapper,						0x0046D845, 2);
	}
}