#include "update_status_effects.h"
#include "../hook_tools.h"

namespace
{
	// Note: acid spores management is normally done in function @ 004F42C0, jumped in at the end of this one, but the hook merges both
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
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_always_detected(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_is_clean_detector_wrapper()
	{
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_clean_detector(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) refresh_carryable_sprite_wrapper()
	{
		static Sprite* sprite;
		__asm {
			MOV sprite, EAX
			PUSHAD
		}
		hooks::refresh_carryable_sprite(sprite);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_carryable_sprite_flags_wrapper()
	{
		static Sprite* sprite;
		__asm {
			MOV sprite, EAX
			PUSHAD
		}
		hooks::update_carryable_sprite_flags(sprite);
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
		jump_patch(update_status_effects_wrapper,			0x00492F70, 5);
		jump_patch(ai_burrower_wrapper,						0x004A1340, 4);
		jump_patch(unit_is_always_detected_wrapper,			0x00491810, 3);
		jump_patch(unit_is_clean_detector_wrapper,			0x00403430,	6);
		jump_patch(refresh_carryable_sprite_wrapper,		0x00497750,	0);
		jump_patch(update_carryable_sprite_flags_wrapper,	0x004981B0,	0);
	}
}