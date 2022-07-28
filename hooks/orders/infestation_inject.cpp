#include "infestation.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) unit_can_infest_wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}
		result = hooks::unit_can_infest(unit) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_can_be_infested_wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EDX
		}
		result = hooks::unit_can_be_infested(unit) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_morph_into_infested_order_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}
		hooks::unit_morph_into_infested_order(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) unit_infest_order_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}
		hooks::unit_infest_order(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_infestation_hooks()
	{
		jump_patch(unit_can_infest_wrapper,					0x00402750);
	//	jump_patch(unit_can_be_infested_wrapper,			0x00402210);
	//	jump_patch(unit_infest_order_wrapper,				0x004EA290);
	//	jump_patch(unit_morph_into_infested_order_wrapper,	0x004EA4C0);
	}
}