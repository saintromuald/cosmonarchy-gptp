#include "sight_range.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_sight_range_wrapper() // 004E5B40
	{
		static Unit* unit;
		static b32 is_for_spellcasting;
		static u32 sightRange;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDX
			MOV EAX, [EBP + 0x08]
			MOV is_for_spellcasting, EAX
			PUSHAD
		}
		sightRange = hooks::get_sight_range(unit, is_for_spellcasting != 0);
		__asm {
			POPAD
			MOV EAX, sightRange
			POP EBP
			RETN 0x04
		}
	}
	void __declspec(naked) reveal_sight_at_location_wrapper() // 004806F0
	{
		static u32 sight_range;
		static u32 flags;
		static s32 x_pos;
		static s32 y_pos;
		static b32 is_air_unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV x_pos, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y_pos, EAX
			MOV EAX, [EBP + 0x10]
			MOV is_air_unit, EAX
			MOV flags, ECX
			MOV sight_range, EAX
			PUSHAD
		}
		hooks::reveal_sight_at_location(sight_range, flags, x_pos, y_pos, is_air_unit);
		__asm {
			POPAD
			POP EBP
			RETN 0x0C
		}
	}
}

namespace hooks
{
	void inject_sight_range_hooks()
	{
		jump_patch(get_sight_range_wrapper,				0x004E5B40, 4);
	//	jump_patch(reveal_sight_at_location_wrapper,	0x004806F0, 0);
	}
}