#include "recharge_shields.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) unit_can_recharge_shields_wrapper() // 00493520
	{
		static Unit* target;
		static Unit* battery;
		static b32 result;
		__asm {
			MOV battery, EDI
			MOV target, EAX
			PUSHAD
		}
		result = hooks::unit_can_recharge_shields(target, battery);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) order_recharge_shields_wrapper() // 00493DD0
	{
		static Unit* unit;
		__asm {
			MOV unit, EDI
			PUSHAD
		}
		hooks::order_recharge_shields(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_recharge_shields_hooks()
	{
		jump_patch(unit_can_recharge_shields_wrapper,	0x00493520, 1);
	//	jump_patch(order_recharge_shields_wrapper,		0x00493DD0, 1);
	}
}