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

	void __declspec(naked) replace_unit_with_id_wrapper() // 0049FED0
	{
		static Unit* unit;
		static u32 new_id;
		static u8 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV new_id, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::replace_unit_with_id(unit, new_id);
		__asm
		{
			POPAD
			MOVZX AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) update_unit_strength_and_apply_default_orders_wrapper() // 0049FA40
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::update_unit_strength_and_apply_default_orders(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) initialize_unit_base_wrapper() // 004A0320
	{
		static Unit* unit;
		static u32 unit_id;
		static u32 pos_x;
		static u32 pos_y;
		static u32 player;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV pos_x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV pos_y, EAX
			MOV EAX, [EBP + 0x10]
			MOV player, EAX
			POP EAX
			MOV unit, ECX
			MOV unit_id, EAX
			PUSHAD
		}
		result = hooks::initialize_unit_base(unit, unit_id, pos_x, pos_y, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}
}

namespace hooks
{
	void inject_update_unit_state_hooks()
	{
		jump_patch(update_unit_energy_wrapper,								0x004EB4B0, 0);
		jump_patch(update_unit_state_wrapper,								0x004EC290, 1);
		jump_patch(update_units_wrapper,									0x004ED000, 5);
		jump_patch(update_burrowing_cloaking_units_wrapper,					0x00492EB0, 3);
	//	jump_patch(do_cloak_wrapper,										0x0049B440, 5);
		jump_patch(progress_unit_frame_wrapper,								0x004ECF70, 3);
		jump_patch(sprite_to_iscript_loop_wrapper,							0x00497920, 0);
		jump_patch(replace_unit_with_id_wrapper,							0x0049FED0, 1);
		jump_patch(update_unit_strength_and_apply_default_orders_wrapper,	0x0049FA40, 1);
	//	jump_patch(initialize_unit_base_wrapper,							0x004A0320, 0);
	}
}