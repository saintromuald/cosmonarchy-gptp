#include "recharge_shields.h"
#include "../hook_tools.h"

namespace
{

	void __declspec(naked) unitCanRechargeShieldsWrapper()
	{

		static Unit* target;
		static Unit* battery;
		static b32 result;

		__asm {
			MOV battery, EDI
			MOV target, EAX
			PUSHAD
		}

		result = hooks::unitCanRechargeShieldsHook(target, battery);

		__asm {
			POPAD
			MOV EAX, result
			RETN
		}

	}

	//Inject with call_patch
	void __declspec(naked) orderRechargeShieldsWrapper()
	{

		static Unit* unit;

		__asm {
			MOV unit, EDI
			PUSHAD
		}

		hooks::orderRechargeShieldsHook(unit);

		__asm {
			POPAD
			RETN
		}

	}

} //unnamed namespace

namespace hooks
{

	void injectRechargeShieldsHooks()
	{
		jump_patch(unitCanRechargeShieldsWrapper, 0x00493520, 1);
		//jump_patch(orderRechargeShieldsWrapper,	0x00493DD0, 1);
	}

} //hooks
