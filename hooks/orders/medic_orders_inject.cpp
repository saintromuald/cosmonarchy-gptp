#include "medic_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_medic_heal2_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_medic_heal2(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_heal_move_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_heal_move(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) orders_medic_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_medic(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_medic_heal1_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_medic_heal1(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_acquire_heal_target_wrapper()
	{
		static Unit* cleric;
		static Unit* target;
		static u32 result;
		__asm
		{
			MOV cleric, EBX
			MOV target, ESI
			PUSHAD
		}
		result = hooks::ai_acquire_heal_target(cleric, target);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) do_medic_heal_wrapper() // 00463C40
	{
		static Unit* medic;
		static Unit* target;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV target, EAX
			POP EAX
			MOV medic, EAX
			PUSHAD
		}
		result = hooks::do_medic_heal(medic, target);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_medic_orders_hooks()
	{
		jump_patch(orders_medic_heal2_wrapper,		0x00463740, 1);
		jump_patch(orders_heal_move_wrapper,		0x004637B0, 1);
		jump_patch(orders_medic_wrapper,			0x00463900, 1);
		jump_patch(orders_medic_heal1_wrapper,		0x00464180, 1);
		jump_patch(ai_acquire_heal_target_wrapper,	0x00463530, 1);
		jump_patch(do_medic_heal_wrapper,			0x00463C40, 1);
	}
}