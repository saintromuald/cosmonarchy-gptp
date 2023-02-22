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

	const u32 UpdateSpriteImagesForCloak_98AC1 = 0x00498AC1;
	void __declspec(naked) update_sprite_images_for_cloak_patch_498ABB() {
		__asm {
			AND		DL, 0xFD
			CMP		word ptr[ECX + 8], 0xD3
			JE		offset isWarpAnchor
			INC		DL
			isWarpAnchor:
			ADD		DL, 6
			JMP		UpdateSpriteImagesForCloak_98AC1
		}
	}

	const u32 DoCloak_9B4CC = 0x0049B4CC;
	void __declspec(naked) do_cloak_patch_49B4C6() {
		__asm {
			PUSH EDI
			MOV	EDI, [EDI + 0x18]
			CMP	word ptr[EDI + 8], 0xD3
			POP EDI
			JE	offset isWarpAnchor
			OR	ECX, 0x200
			isWarpAnchor:
			JMP	DoCloak_9B4CC
		}
	}
}

namespace hooks
{
	void inject_update_status_effects_hooks()
	{
		jump_patch(update_status_effects_wrapper,				0x00492F70, 5);
		jump_patch(ai_burrower_wrapper,							0x004A1340, 4);
		jump_patch(unit_is_always_detected_wrapper,				0x00491810, 3);
		jump_patch(unit_is_clean_detector_wrapper,				0x00403430,	6);
		jump_patch(refresh_carryable_sprite_wrapper,			0x00497750,	0);
		jump_patch(update_carryable_sprite_flags_wrapper,		0x004981B0,	0);

		// From DarkenedFantasies - for fixing cloak-warp anchor interactions.
		jump_patch(update_sprite_images_for_cloak_patch_498ABB,	0x00498ABB, 1);
	//	jump_patch(do_cloak_patch_49B4C6,						0x0049B4C6, 1); // Achieved with a nops in initialize.cpp
	}
}