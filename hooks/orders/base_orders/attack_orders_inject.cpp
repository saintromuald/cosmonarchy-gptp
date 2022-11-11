#include "attack_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_ReaverAttack1Wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_reaver_attack(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_CarrierAttack1Wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_carrier_attack(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_TurretAttackWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_turret_attack(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_AttackFixedRangeWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_attack_fixed_range(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_SapUnitWrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_sap_unit(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_SapLocationWrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_sap_location(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_attack_move_ep_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_attack_move_ep(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_attack_move_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::orders_attack_move(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_AttackUnitWrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::orders_attack_unit(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_strafe_unit2_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_strafe_unit2(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) set_interceptors_to_standby_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		hooks::set_interceptors_to_standby(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_attack_orders_hooks()
	{
	//	jump_patch(orders_ReaverAttack1Wrapper,			0x00465690, 2);
	//	jump_patch(orders_CarrierAttack1Wrapper,		0x00465950, 0);
	//	jump_patch(orders_TurretAttackWrapper,			0x00477980, 1);
	//	jump_patch(orders_AttackFixedRangeWrapper,		0x00477D00, 0);
	//	jump_patch(orders_SapUnitWrapper,				0x004788E0, 2);
	//	jump_patch(orders_SapLocationWrapper,			0x00478A40, 3);
		jump_patch(orders_attack_move_ep_wrapper,		0x00478DE0, 1);
		jump_patch(orders_attack_move_wrapper,			0x00479040, 0);
	//	jump_patch(orders_AttackUnitWrapper,			0x00479BD0, 2);
		jump_patch(orders_strafe_unit2_wrapper,			0x00465E00, 2);
		jump_patch(set_interceptors_to_standby_wrapper,	0x00465780, 1);
	}
}