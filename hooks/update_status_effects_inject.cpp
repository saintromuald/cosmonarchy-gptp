#include "update_status_effects.h"
#include "../hook_tools.h"

namespace
{
	// note: acid spores management is normally done in function @ 004F42C0, jumped in at the end of this one, but the hook merge both
	void __declspec(naked) updateStatusEffectsWrapper()
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

	void __declspec(naked) AiBurrower_Wrapper()
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
}

namespace hooks
{
	void inject_update_status_effects_hooks()
	{
		jump_patch(updateStatusEffectsWrapper,	0x00492F70, 5);
		jump_patch(AiBurrower_Wrapper,			0x004A1340, 4);
	}
}