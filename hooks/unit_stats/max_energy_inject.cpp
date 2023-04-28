#include "max_energy.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_unit_max_energy_wrapper()
	{
		static Unit* unit;
		static u16 max_energy;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		max_energy = hooks::get_unit_max_energy(unit);
		__asm
		{
			POPAD
			MOVZX EAX, max_energy
			RETN
		}
	}
}

namespace hooks
{
	void inject_unit_max_energy_hooks()
	{
		jump_patch(get_unit_max_energy_wrapper, 0x00491870, 7);
	}
}