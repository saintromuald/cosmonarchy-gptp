#include "liftland.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_building_land_wrapper() // 00464360
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x10
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_building_land(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) orders_building_liftoff_wrapper() // 004649B0
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV unit, ESI
			PUSHAD
		}
		hooks::orders_building_liftoff(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}
}

namespace hooks
{
	void inject_lift_land_hooks()
	{
		jump_patch(orders_building_land_wrapper,	0x00464360, 1);
		jump_patch(orders_building_liftoff_wrapper,	0x004649B0, 1);
	}
}