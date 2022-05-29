//Injector source file for the Stop/HoldPosition Orders hook module.
#include "stopholdpos_orders.h"
#include <hook_tools.h>

namespace
{

	void __declspec(naked) orders_MedicHoldPositionWrapper()
	{

		static Unit* unit;

		__asm {
			MOV unit, ESI
			PUSHAD
		}

		hooks::orders_MedicHoldPosition(unit);

		__asm {
			POPAD
			RETN
		}

	}

	;

	void __declspec(naked) orders_ReaverStopWrapper()
	{

		static Unit* unit;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}

		hooks::orders_ReaverStop(unit);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}

	}

	;

	void __declspec(naked) orders_GuardWrapper()
	{

		static Unit* unit;

		__asm {
			MOV unit, ESI
			PUSHAD
		}

		hooks::orders_Guard(unit);

		__asm {
			POPAD
			RETN
		}

	}

	;

	void __declspec(naked) orders_player_guard_wrapper()
	{

		static Unit* unit;

		__asm {
			MOV unit, EAX
			PUSHAD
		}

		hooks::orders_player_guard(unit);

		__asm {
			POPAD
			RETN
		}

	}

	;

}//unnamed namespace

namespace hooks
{

	void injectStopHoldPosOrdersHooks()
	{
		//jump_patch(orders_MedicHoldPositionWrapper, 0x00464050, 0);
		//jump_patch(orders_ReaverStopWrapper, 0x004654B0, 1);
		//jump_patch(orders_GuardWrapper, 0x00475B90, 2);
		jump_patch(orders_player_guard_wrapper, 0x004774A0, 3);
	}

} //hooks
