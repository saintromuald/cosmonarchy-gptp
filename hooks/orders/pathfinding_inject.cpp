#include "pathfinding.h"
#include <hook_tools.h>
#include <SCBW/api.h>

namespace
{
	void __declspec(naked) move_to_target_wrapper() // 004EB720
	{
		static s32 result;
		static Unit* target;
		static Unit* unit;
		__asm
		{
			MOV target, EDI
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::move_to_target(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_active_tile_flag_wrapper() // 00472F80
	{
		static b32 result;
		static s16 x;
		static s16 y;
		static s32 includedFlags;
		static s32 excludedFlags;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOVZX EAX, [EBP + 0x08]
			MOV x, AX
			MOV EAX, [EBP + 0x0C]
			MOV includedFlags, EAX
			MOV EAX, [EBP + 0x10]
			MOV excludedFlags, EAX
			POP EAX
			MOV y, AX
			PUSHAD
		}
		result = hooks::get_active_tile_flag(y, x, includedFlags, excludedFlags);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) unit_has_path_to_dest_wrapper() // 0049CB60
	{
		static Unit* unit;
		static s32 x;
		static s32 y;
		static bool result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_has_path_to_dest(unit, x, y);
		__asm
		{
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) unit_has_path_to_unit_wrapper() // 0049CBB0
	{
		static Unit* pather;
		static Unit* target;
		static bool result;
		__asm
		{
			MOV pather, EAX
			MOV target, ECX
			PUSHAD
		}
		result = hooks::unit_has_path_to_unit(pather, target);
		_asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) set_repulse_angle_wrapper() // 0049CBB0
	{
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		hooks::set_repulse_angle(unit);
		_asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) repulse_unit_wrapper() // 004535A0
	{
		static u16 result;
		static Unit* unit;
		static void* LOLRSK8S_EQUIPPED;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV LOLRSK8S_EQUIPPED, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::repulse_unit(unit, LOLRSK8S_EQUIPPED);
		_asm
		{
			POPAD
			MOV AX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_pathfinding_hooks()
	{
		jump_patch(is_point_at_unit_border,			0x0042DA90, 1);
		jump_patch(move_to_target_wrapper,			0x004EB720, 0);
	//	jump_patch(get_active_tile_flag_wrapper,	0x00472F80, 1);
		jump_patch(unit_has_path_to_dest_wrapper,	0x0049CB60, 1);
		jump_patch(unit_has_path_to_unit_wrapper,	0x0049CBB0, 0);
		jump_patch(set_repulse_angle_wrapper,		0x00453300, 5);
		jump_patch(repulse_unit_wrapper,			0x004535A0, 1);
	}
}