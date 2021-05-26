#include "supply_hooks.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) getSuppliesAvailable_Wrapper()
	{
		static u8 playerId;
		static s32 race;
		static u32 result;
		__asm
		{
			MOV race, ECX
			MOV playerId, AL
			PUSHAD
		}
		result = hooks::getSuppliesAvailable(playerId, race);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectSupplyHooks()
	{
		jmpPatch(getSuppliesAvailable_Wrapper, 0x00488900, 1);
	}
}