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

	void __declspec(naked) update_building_placement_state_units_wrapper() // 00473720
	{
		static s32 result;
		static s32 y_tile;
		static Unit* builder;
		static s32 x_tile;
		static u8 player;
		static u16 unit_id;
		static Point16 size_wh;
		static s32 placement_state_entry;
		static char dont_ignore_reacting;
		static char also_invisible;
		static s32 without_vision;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV builder, EAX
			MOV EAX, [EBP + 0x0C]
			MOV x_tile, EAX
			MOV EAX, [EBP + 0x10]
			MOV player, AL
			MOV EAX, [EBP + 0x14]
			MOV unit_id, AX
			MOV EAX, [EBP + 0x18]
			MOV size_wh, EAX
			MOV EAX, [EBP + 0x1C]
			MOV placement_state_entry, EAX
			MOV EAX, [EBP + 0x20]
			MOV dont_ignore_reacting, AL
			MOV EAX, [EBP + 0x24]
			MOV also_invisible, AL
			MOV EAX, [EBP + 0x28]
			MOV without_vision, EAX
			POP EAX
			MOV y_tile, EAX
			PUSHAD
		}
		hooks::update_building_placement_state_units(y_tile, builder, x_tile, player, unit_id, size_wh, placement_state_entry, dont_ignore_reacting, also_invisible, without_vision);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN 0x24
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
//		jump_patch(update_building_placement_state_units_wrapper,		0x00473720, 1);
	}
}