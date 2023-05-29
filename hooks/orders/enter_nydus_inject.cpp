#include "enter_nydus.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) canEnterNydusCanalWrapper()
	{
		static Unit* unit;
		static Unit* nydus;
		static b32 result;
		__asm
		{
			MOV unit, EAX
			MOV nydus, EDX
			PUSHAD
		}
		if (hooks::can_enter_nydus(unit, nydus))
		{
			result = 1;
		} else {
			result = 0;
		}
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) enterNydusCanal_EffectWrapper()
	{
		static Unit* unit;
		static Unit* nydus;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, ECX
			MOV nydus, EAX
			PUSHAD
		}
		hooks::enter_nydus_effect(unit, nydus);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_EnterNydusCanalWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_enter_nydus(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_enter_nydus_hooks()
	{
		jump_patch(canEnterNydusCanalWrapper,			0x004E8C20, 2);
		jump_patch(enterNydusCanal_EffectWrapper,		0x004EA180, 1);
		jump_patch(orders_EnterNydusCanalWrapper,		0x004EA3E0, 0);
	}
}
