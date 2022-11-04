#include "update_status_effects.h"
#include "../hook_tools.h"

namespace
{
	// note: acid spores management is normally done in function @ 004F42C0, jumped in at the end of this one, but the hook merge both
	void __declspec(naked) update_status_effects_wrapper()
	{

		static Unit* unit;

		__asm {
			MOV unit, EAX
			PUSHAD
		}

		hooks::update_status_effects(unit);

		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_burrower_wrapper()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::ai_burrower(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) unit_is_always_detected_wrapper()
	{
		static bool result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_always_detected(unit);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_update_status_effects_hooks()
	{
		jump_patch(update_status_effects_wrapper,	0x00492F70, 5);
		jump_patch(ai_burrower_wrapper,				0x004A1340, 4);
		jump_patch(unit_is_always_detected_wrapper,	0x00491810, 3);
	}
}