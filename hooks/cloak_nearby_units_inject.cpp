#include "cloak_nearby_units.h"
#include <hook_tools.h>

void __declspec(naked) cloakNearbyUnitsWrapper()
{
	static Unit* unit;
	__asm
	{
		PUSH EBP
		MOV EBP, ESP
		MOV unit, EDI
		PUSHAD
	}
	hooks::cloak_nearby_units(unit);
	__asm
	{
		POPAD
		MOV ESP, EBP
		POP EBP
		RETN
	}
}

namespace hooks
{
	void inject_cloak_nearby_units_hooks()
	{
		jump_patch(cloakNearbyUnitsWrapper, 0x00491C20, 1);
	}
}