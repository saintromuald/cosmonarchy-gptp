#include "attack_orders.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) orders_reaver_attack_wrapper() {
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

	void __declspec(naked) orders_carrier_attack_wrapper() {
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

	void __declspec(naked) orders_turret_attack_wrapper() {
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

	void __declspec(naked) orders_attack_fixed_range_wrapper() {
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

	void __declspec(naked) orders_sap_unit_wrapper() {
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

	void __declspec(naked) orders_sap_location_wrapper() {
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

	void __declspec(naked) orders_attack_move_ep_wrapper() {
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

	void __declspec(naked) orders_attack_move_wrapper() {
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

	void __declspec(naked) orders_attack_unit_wrapper() {
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

	void __declspec(naked) orders_strafe_unit2_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_strafe_unit2(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) set_interceptors_to_standby_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::set_interceptors_to_standby(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_strafe_unit_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_strafe_unit(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_next_strafe_position_wrapper() {
		static Unit* unit;
		static u8 strafe_type;
		static Point16 position;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV position, EAX
			POP EAX
			MOV strafe_type, CL
			MOV unit, EAX
			PUSHAD
		}
		hooks::get_next_strafe_position(unit, strafe_type, position);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) orders_carrier_idle_wrapper() { // 004666A0
		static Unit* unit;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_carrier_idle(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) order_attack_behavior_wrapper() { // 00478540
		static s32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::order_attack_behavior(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) attack_move_react_to_attack_wrapper() { // 00478370
		static b32 result;
		static Unit* unit;
		static u32 order_id;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV order_id, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::attack_move_react_to_attack(unit, order_id);
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

namespace hooks {
	void inject_attack_orders_hooks() {
	//	jump_patch(orders_reaver_attack_wrapper,		0x00465690, 2);
	//	jump_patch(orders_carrier_attack_wrapper,		0x00465950, 0);
	//	jump_patch(orders_turret_attack_wrapper,		0x00477980, 1);
	//	jump_patch(orders_attack_fixed_range_wrapper,	0x00477D00, 0);
	//	jump_patch(orders_sap_unit_wrapper,				0x004788E0, 2);
	//	jump_patch(orders_sap_location_wrapper,			0x00478A40, 3);
		jump_patch(orders_attack_move_ep_wrapper,		0x00478DE0, 1);
		jump_patch(orders_attack_move_wrapper,			0x00479040, 0);
	//	jump_patch(orders_attack_unit_wrapper,			0x00479BD0, 2);
		jump_patch(orders_strafe_unit2_wrapper,			0x00465E00, 2);
	//	jump_patch(set_interceptors_to_standby_wrapper,	0x00465780, 1);
	//	jump_patch(orders_strafe_unit_wrapper,			0x00465F60, 0);
	//	jump_patch(get_next_strafe_position_wrapper,	0x00465D30, 0);
		jump_patch(orders_carrier_idle_wrapper,			0x004666A0, 0);
		jump_patch(order_attack_behavior_wrapper,		0x00478540, 0);
		jump_patch(attack_move_react_to_attack_wrapper,	0x00478370, 0);
	}
}