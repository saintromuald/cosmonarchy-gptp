#include "selection_extender.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) select_dual_birth_units_wrapper() { // 0045D040
		static u32 result;
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::select_dual_birth_units(unit);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) is_selected_unit_group_enabled_wrapper() { // 0049A2C0
		static bool result;
		__asm {
			PUSHAD
		}
		result = hooks::is_selected_unit_group_enabled();
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) select_unit_initial_wrapper() { // 0049B690
		static char result;
		__asm {
			PUSHAD
		}
		result = hooks::select_unit_initial();
		__asm {
			POPAD
			MOVZX AL, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_selection_extender_hooks()
	{
		jump_patch(select_dual_birth_units_wrapper,			0x0045D040, 1);
	//	jump_patch(is_selected_unit_group_enabled_wrapper,	0x0049A2C0, 1); // Causes issues with quick placebuilding somehow
	//	jump_patch(select_unit_initial_wrapper,				0x0049B690, 1); // Not finished
	}
}