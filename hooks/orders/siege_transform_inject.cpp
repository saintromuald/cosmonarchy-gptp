#include "siege_transform.h"
#include <hook_tools.h>

void __declspec(naked) unitTransformIntoTankModeOrder()
{
	static Unit* unitTransforming;
	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unitTransforming, EAX
	}
	hooks::ordersTankMode(unitTransforming);
	__asm {
		POPAD
		RETN
	}
}

void __declspec(naked) unitTransformIntoSiegeModeOrder()
{
	static Unit* unitTransforming;
	__asm {
		PUSHAD
		MOV EBP, ESP
		MOV unitTransforming, EDI
	}
	hooks::ordersSiegeMode(unitTransforming);
	__asm {
		POPAD
		RETN
	}
}

namespace hooks
{
	void inject_siege_transform_hooks()
	{
		jump_patch(unitTransformIntoTankModeOrder,	0x00464AE0, 1);
		jump_patch(unitTransformIntoSiegeModeOrder, 0x00464BD0, 1);
	}
}