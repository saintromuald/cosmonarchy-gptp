#include "trap_hooks.h"
#include "hook_tools.h"
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) unitIsInstallationAsset_Wrapper()
	{
		static u32 id;
		static u32 result;
		__asm
		{
			MOV id, EAX
			PUSHAD
		}
		result = hooks::unitIsInstallationAsset(id);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unitIsFloorTrap_Wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unitIsFloorTrap(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) disableTrap_Wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		result = hooks::disableTrap(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) updateUnitStrengthAndApplyDefaultOrders_Wrapper()
	{
		static Unit* a1;
		static u32 result;
		__asm
		{
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::updateUnitStrengthAndApplyDefaultOrdersHook(a1);
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
	void injectTrapHooks()
	{
		jmpPatch(unitIsInstallationAsset_Wrapper, 0x00401430, 3);
		jmpPatch(unitIsFloorTrap_Wrapper, 0x0047B070, 3);
	}
}