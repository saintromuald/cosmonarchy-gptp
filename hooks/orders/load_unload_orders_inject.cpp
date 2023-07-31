#include "load_unload_orders.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_pickup_1_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_pickup_1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_pickup_3_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_pickup_3(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_pickup_2_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_pickup_2(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_move_unload_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_move_unload(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) orders_pickup4_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_pickup_4(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_enter_transport_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_enter_transport(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_unload_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_unload(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) can_enter_transport_wrapper()
	{
		static Unit* transport;
		static Unit* unit;
		static bool result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV transport, EAX
			PUSHAD
		}
		result = hooks::can_enter_transport(transport, unit);
		__asm {
			POPAD
			MOVZX EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_load_unload_orders_hooks()
	{
		jump_patch(orders_pickup_1_wrapper,			0x004E7300, 1);
		jump_patch(orders_pickup_3_wrapper,			0x004E73B0, 1);
		jump_patch(orders_pickup_2_wrapper,			0x004E75D0, 1);
		jump_patch(orders_move_unload_wrapper,		0x004E7700, 1);
	//	jump_patch(orders_pickup4_wrapper,			0x004E7B70, 1);
		jump_patch(orders_enter_transport_wrapper,	0x004E7CF0, 0);
		jump_patch(orders_unload_wrapper,			0x004E80D0, 1);
		jump_patch(can_enter_transport_wrapper,		0x004E6E00, 1);
	}
}