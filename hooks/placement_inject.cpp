#include "placement.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) init_palcement_box_pos_wrapper()
	{
		static s32 unused;
		static Point16* placement;
		static s32 result;

		__asm
		{
			MOV unused, ECX
			MOV placement, EDX
			PUSHAD
		}
		result = hooks::init_placement_box_pos(unused, placement);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) draw_placement_box_wrapper()
	{
		static Point16 ecx0;
		static u8 result;

		__asm
		{
			MOV ecx0, ECX
			PUSHAD
		}
		result = hooks::draw_placement_box(ecx0);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) update_placement_state_for_placement_box_wrapper() // 0048DCE0
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::update_placement_state_for_placement_box();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) is_tile_blocked_by_wrapper() { // 00473300
		static s32 result;
		static Unit** unitArray;
		static Unit* builder;
		static s32 x_tile;
		static s32 y_tile;
		static s8 dontIgnoreReacting;
		static s8 alsoInvisible;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV builder, EAX
			MOV EAX, [EBP + 0x0C]
			MOV x_tile, EAX
			MOV EAX, [EBP + 0x10]
			MOV y_tile, EAX
			MOV EAX, [EBP + 0x14]
			MOV dontIgnoreReacting, AL
			MOV EAX, [EBP + 0x18]
			MOV alsoInvisible, AL
			POP EAX
			MOV unitArray, EAX
			PUSHAD
		}
		result = hooks::is_tile_blocked_by(unitArray, builder, x_tile, y_tile, dontIgnoreReacting, alsoInvisible);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}
}

namespace hooks
{
	void injectPlacementHooks()
	{
		jump_patch(init_palcement_box_pos_wrapper,						0x0048D660, 0);
		jump_patch(draw_placement_box_wrapper,							0x0048DDC0, 4);
//		jump_patch(update_placement_state_for_placement_box_wrapper,	0x0048DCE0, 0);
		jump_patch(is_tile_blocked_by_wrapper,							0x00473300, 0);
	}
}