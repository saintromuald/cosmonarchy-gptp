#include "max_energy.h"
#include <hook_tools.h>

extern const u32 Func_GetMaxEnergy; // Defined in Unit.cpp
namespace
{
	void __declspec(naked) getUnitMaxEnergyWrapper()
	{
		static Unit* unit;
		static u16 maxEnergy;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		maxEnergy = hooks::getUnitMaxEnergyHook(unit);
		__asm {
			POPAD
			MOVZX EAX, maxEnergy
			RETN
		}
	}
}

namespace hooks
{
	void inject_unit_max_energy_hooks()
	{
		jump_patch(getUnitMaxEnergyWrapper, Func_GetMaxEnergy, 7);
	}
}