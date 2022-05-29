#include "medic_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_MedicHeal2Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_MedicHeal2(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_HealMoveWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_HealMove(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) orders_MedicWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_Medic(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_MedicHeal1Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_MedicHeal1(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_AcquireHealTarget_Wrapper()
	{
		static Unit* a1;
		static Unit* a2;
		static u32 result;
		__asm
		{
			MOV a1, EBX
			MOV a2, ESI
			PUSHAD
		}
		result = hooks::AI_AcquireHealTarget(a1, a2);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) doMedicHeal_Wrapper()
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
		result = hooks::doMedicHeal_Func(medic, target);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

}

namespace hooks
{

	void injectMedicOrdersHooks()
	{
		jump_patch(orders_MedicHeal2Wrapper,		0x00463740, 1);
		jump_patch(orders_HealMoveWrapper,		0x004637B0, 1);
		jump_patch(orders_MedicWrapper,			0x00463900, 1);
		jump_patch(orders_MedicHeal1Wrapper,		0x00464180, 1);
		jump_patch(AI_AcquireHealTarget_Wrapper,	0x00463530, 1);
		jump_patch(doMedicHeal_Wrapper,			0x00463C40, 1);
	}

} //hooks