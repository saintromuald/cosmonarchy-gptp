#include <SCBW/api.h>
#include "sfxhooks.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_unit_response_id_wrapper() // 0048EA20
	{
		static s32 unit_id;
		static s32 last_sound_id;
		static s32 result;
		__asm {
			MOV unit_id, EAX
			MOV last_sound_id, EDX
			PUSHAD
		}
		result = hooks::get_unit_response_id(unit_id, last_sound_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) play_yes_unit_response_wrapper() // 0048F4D0
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::play_yes_unit_response(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_random_yes_unit_response_wrapper() // 0048EAB0
	{
		static Unit* unit; // Avoiding compiler errors.
		static u32 unit_id;
		static u32 result;
		__asm {
			MOV unit_id, ECX
			PUSHAD
		}
		result = hooks::get_random_yes_unit_response(unit_id, unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_sfx_hooks()
	{
		jump_patch(get_unit_response_id_wrapper,			0x0048EA20, 1);
		jump_patch(play_yes_unit_response_wrapper,			0x0048F4D0, 3);
	//	jump_patch(get_random_yes_unit_response_wrapper,	0x0048EAB0, 0); // No need to inject - only called by play_yes_unit_response. -Pr0nogo
	}
}