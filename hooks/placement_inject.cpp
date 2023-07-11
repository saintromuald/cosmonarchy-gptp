#include "placement.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) init_placement_box_pos_wrapper() // 0048D660
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

	void __declspec(naked) draw_placement_box_wrapper() // 0048DDC0
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

	void __declspec(naked) update_creep_building_placement_state_wrapper() { // 00473010
		static u32 result;
		static u8 player;
		static u16 x_tile;
		static u16 y_tile;
		static Point16 size_wh;
		static u32 placement_state_entry;
		static u32 check_vision;
		static u32 check_vision_2;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x_tile, AX
			MOV EAX, [EBP + 0x0C]
			MOV y_tile, AX
			MOV EAX, [EBP + 0x10]
			MOV size_wh, EAX
			MOV EAX, [EBP + 0x14]
			MOV placement_state_entry, EAX
			MOV EAX, [EBP + 0x18]
			MOV check_vision, EAX
			MOV EAX, [EBP + 0x1C]
			MOV check_vision_2, EAX
			POP EAX
			MOV player, AL
			PUSHAD
		}
		result = hooks::update_creep_building_placement_state(player, x_tile, y_tile, size_wh, placement_state_entry, check_vision, check_vision_2);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x18
		}
	}

	void __declspec(naked) update_building_placement_state_map_tile_flags_wrapper() { // 00473A10
		static u32 result;
		static u16 unit_id;
		static u16 tile_x;
		static u16 tile_y;
		static u8 player;
		static Point16 size_wh;
		static u32 placement_state_entry;
		static u32 check_creep;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV tile_x, AX
			MOV EAX, [EBP + 0x0C]
			MOV tile_y, AX
			MOV EAX, [EBP + 0x10]
			MOV player, AL
			MOV EAX, [EBP + 0x14]
			MOV size_wh, EAX
			MOV EAX, [EBP + 0x18]
			MOV placement_state_entry, EAX
			MOV EAX, [EBP + 0x1C]
			MOV check_creep, EAX
			POP EAX
			MOV unit_id, AX
			PUSHAD
		}
		result = hooks::update_building_placement_state_map_tile_flags(unit_id, tile_x, tile_y, player, size_wh, placement_state_entry, check_creep);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x18
		}
	}

	void __declspec(naked) start_building_wrapper() // 00467030
	{
		static s32 result;
		static Unit* builder;
		__asm
		{
			MOV builder, EAX
			PUSHAD
		}
		result = hooks::start_building(builder);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) place_building_refinery_check_wrapper() { // 0048E1E0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::place_building_refinery_check(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) place_building_refinery_proc_wrapper() { // 0048E430
		__asm {
			PUSHAD
		}
		hooks::place_building_refinery_proc();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) place_building_proc_wrapper() { // 0048E6E0
		__asm {
			PUSHAD
		}
		hooks::place_building_proc();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) unit_id_is_refinery_wrapper() { // 0047AFE0
		static b32 result;
		static u16 unit_id;
		__asm {
			MOV unit_id, AX
			PUSHAD
		}
		result = hooks::unit_id_is_refinery(unit_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) issue_place_building_order_wrapper() { // 0048E730
		static char result;
		static char* unused;
		static u16 unit_id;
		static u8 order_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV order_id, AL
			POP EAX
			MOV unit_id, SI
			MOV unused, EDI
			PUSHAD
		}
		result = hooks::issue_place_building_order(unused, unit_id, order_id);
		__asm {
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) play_building_placement_sound_wrapper() { // 0048F630
		Unit* active_unit;
		__asm {
			MOV active_unit, EAX
			PUSHAD
		}
		hooks::play_building_placement_sound(active_unit);
		__asm {
			POPAD
			RETN
		}
	}

	// Patches \\

	const u32 TTBaseUnitType_6E19A = 0x0046E19A;
	void __declspec(naked) TTBaseUnitType_patch_6E1A6() {
		__asm {
			LEA		ESI, [EAX * 2 + units_req + 2]
			MOV		DX, [ESI]
			CMP		DX, CX//UnitId
			JE		offset finishChecking
			LEA		ESI, [EAX * 2 + units_req + 4]
			CMP		word ptr[ESI], 0xFF01//reqOpcode_Or
			JE		offset checkNextItem
			finishChecking :
			MOV		AX, DX
			RET
			checkNextItem :
			ADD		EAX, 3//Go to next requirement.
			JMP		TTBaseUnitType_6E19A
		}
	}

	const u32 do_place_building_checking_8E247 = 0x0048E247;
	const u32 do_place_building_checking_8E26F = 0x0048E26F;
	const u32 do_place_building_checking_8E305 = 0x0048E305;
	void __declspec(naked) do_place_building_checking_patch_8E253() {
		__asm {
			PUSH	EDI
			LEA		EDI, [EAX * 2 + units_req + 2]
			MOV		CX, [EDI]
			CMP		CX, ds: [0x0064088A]//placeBuildingUnitType
			JE		offset finishChecking
			LEA		EDI, [EAX * 2 + units_req + 4]
			CMP		word ptr[EDI], 0xFF01//reqOpcode_Or
			JE		offset checkNextItem
			POP		EDI
			JMP		do_place_building_checking_8E305//No matching IDs.
			finishChecking :
			MOV		AX, CX
			POP		EDI
			JMP		do_place_building_checking_8E26F//Found a match.
			checkNextItem :
			ADD		EAX, 3//Go to next requirement.
			POP		EDI
			JMP		do_place_building_checking_8E247
		}
	}
}

namespace hooks
{
	void inject_placement_hooks()
	{
		jump_patch(init_placement_box_pos_wrapper,							0x0048D660, 0);
		jump_patch(draw_placement_box_wrapper,								0x0048DDC0, 4);
	//	jump_patch(update_placement_state_for_placement_box_wrapper,		0x0048DCE0, 0);
		jump_patch(is_tile_blocked_by_wrapper,								0x00473300, 0);
		jump_patch(update_creep_building_placement_state_wrapper,			0x00473010, 1);
		jump_patch(update_building_placement_state_map_tile_flags_wrapper,	0x00473A10, 1);
		jump_patch(start_building_wrapper,									0x00467030, 1);
		jump_patch(place_building_refinery_check_wrapper,					0x0048E1E0, 3);
		jump_patch(place_building_refinery_proc_wrapper,					0x0048E430, 2);
		jump_patch(place_building_proc_wrapper,								0x0048E6E0, 0);
		jump_patch(unit_id_is_refinery_wrapper,								0x0047AFE0, 1);
		jump_patch(issue_place_building_order_wrapper,						0x0048E730, 8);
	//	jump_patch(play_building_placement_sound_wrapper,					0x0048F630, 3); // Causes landing crash lol

		// DF - Fix Treasury being unable to claim addons
	//	jump_patch(TTBaseUnitType_patch_6E1A6,								0x0046E1A6, 3); //TTBaseUnitType (0x0046E170)
	//	jump_patch(do_place_building_checking_patch_8E253,					0x0048E253, 3); //do_place_building_checking (0x0048E210)
	}
}