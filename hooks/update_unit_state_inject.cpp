#include "update_unit_state.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) update_unit_energy_wrapper() // 004EB4B0
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::update_unit_energy(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_unit_state_wrapper() // 004EC290
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::update_unit_state(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_units_wrapper() // 004ED000
	{
		__asm {
			PUSHAD
		}
		hooks::update_units();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_burrowing_cloaking_units_wrapper() // 00492EB0
	{
		__asm {
			PUSHAD
		}
		hooks::update_burrowing_cloaking_units();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) do_cloak_wrapper() // 0049B440
	{
		static Unit* unit;
		static s32 sound_id;
		__asm {
			MOV sound_id, EBX
			MOV unit, EAX
			PUSHAD
		}
		hooks::do_cloak(unit, sound_id);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) progress_unit_frame_wrapper() // 004ECF70
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		hooks::progress_unit_frame(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) sprite_to_iscript_loop_wrapper() // 00497920
	{
		static Sprite* sprite;
		__asm
		{
			MOV sprite, ESI
			PUSHAD
		}
		hooks::sprite_to_iscript_loop(sprite);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_update_unit_state_hooks()
	{
		jump_patch(update_unit_energy_wrapper,				0x004EB4B0, 0);
		jump_patch(update_unit_state_wrapper,				0x004EC290, 1);
		jump_patch(update_units_wrapper,					0x004ED000, 5);
	//	jump_patch(update_burrowing_cloaking_units_wrapper,	0x00492EB0, 3);
	//	jump_patch(do_cloak_wrapper,						0x0049B440, 5);
		jump_patch(progress_unit_frame_wrapper,				0x004ECF70, 3);
		jump_patch(sprite_to_iscript_loop_wrapper,			0x00497920, 0);
	}
}