#include "bunker_hooks.h"
#include "../hook_tools.h"

namespace
{
	const u32 Hook_UnitCanAttackInsideBunkerYes = 0x004790DC;
	void __declspec(naked) unit_attacks_from_inside_bunker_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::unit_attacks_from_inside_bunker(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) create_bunker_attack_overlay_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::create_bunker_attack_overlay(unit);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_bunker_hooks()
	{
		jump_patch(unit_attacks_from_inside_bunker_wrapper,		0x004790A0, 2);
		jump_patch(create_bunker_attack_overlay_wrapper,		0x00477FD0, 3);
	}
}
