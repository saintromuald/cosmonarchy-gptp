#include "rally_point.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) order_new_unit_to_rally_wrapper()
	{
		static Unit* unit;
		static Unit* factory;
		__asm {
			MOV unit, EAX
			MOV factory, ECX
			PUSHAD
		}
		hooks::order_new_unit_to_rally(unit, factory);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) set_rally_position_wrapper()
	{
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			MOV CX, [EBP + 0x08]
			MOV x, CX
			MOV DX, [EBP + 0x0C]
			MOV y, DX
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) set_rally_unit_wrapper()
	{
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, ECX
			MOV target, EAX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);
		__asm {
			POPAD
			RETN
		}
	}

	// Additional wrappers for uses of the logic that would be hardcoded instead of using the functions
	void __declspec(naked) set_rally_position_wrapper_jump1()
	{
		static const u32 Hook_SetRallyGenericJmpBack1 = 0x00456384;
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			MOV unit, EDI
			MOV x, BX
			MOV y, SI
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack1
		}
	}

	void __declspec(naked) set_rally_position_wrapper_jump2()
	{
		static const u32 Hook_SetRallyGenericJmpBack2 = 0x0049AD77;
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			MOV unit, ESI
			MOV x, CX
			MOV y, DX
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack2
		}
	}

	void __declspec(naked) set_rally_unit_wrapper_jump1()
	{
		static const u32 Hook_SetRallyGenericJmpBack1 = 0x00456384;
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, EDI
			MOV target, EAX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack1
		}
	}

	void __declspec(naked) set_rally_unit_wrapper_jump2()
	{
		static const u32 Hook_SetRallyGenericJmpBack2 = 0x0049AD77;
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, ESI
			MOV target, EBX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);

		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack2
		}
	}

	void __declspec(naked) remove_references_wrapper() { // 0049E590
		static Unit* unit;
		static Unit* dying;
		static Order* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV dying, EAX
			POP EAX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::remove_references(unit, dying);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) right_click_command_wrapper() { // 004560D0
		static s32 x;
		static s32 y;
		static Unit* unit;
		static s32 fow_unit_id;
		static s8 queued;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			MOV EAX, [EBP + 0x10]
			MOV unit, EAX
			MOV EAX, [EBP + 0x14]
			MOV fow_unit_id, EAX
			MOV EAX, [EBP + 0x18]
			MOV queued, AL
			POP EAX
			PUSHAD
		}
		result = hooks::right_click_command(x, y, unit, fow_unit_id, queued);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) btnscond_rally_wrapper() // 00429520
	{
		static u32 unused;
		static u32 player;
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV player, EDX
			MOV unused, ECX
			PUSHAD
		}
		result = hooks::btnscond_rally(unused, player, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) unit_can_have_rally_wrapper() { // 0047B2E0
		static u32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_can_have_rally(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_can_have_rally_original_wrapper() {
		static u32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_can_have_rally_original(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) order_acquire_generic_right_click_unit_proc_2_wrapper() { // 00455E00
		static u32 result;
		static Unit* unit;
		static u32 x_pos;
		static u16 y_pos;
		static Unit** target;
		static u16 fow_unit_id;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV y_pos, AX
			MOV EAX, [EBP + 0x0C]
			MOV target, EAX
			MOV EAX, [EBP + 0x10]
			MOV fow_unit_id, AX
			POP EAX
			MOV x_pos, EDX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::order_acquire_generic_right_click_unit_proc_2(unit, x_pos, y_pos, target, fow_unit_id);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) process_right_click_action_wrapper() { // 00455F20
		static Unit* unit;
		static u32 order_unit_id;
		static u16 y_pos;
		static u32 x_pos;
		static Unit* target;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV x_pos, EAX
			MOV EAX, [EBP + 0x0C]
			MOV target, EAX
			POP EAX
			MOV y_pos, DI
			MOV order_unit_id, ECX
			MOV unit, EAX
			PUSHAD
		}
		hooks::process_right_click_action(unit, order_unit_id, y_pos, x_pos, target);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) get_generic_unit_idle_order_wrapper() { // 00455760
		static u8 result;
		static u32 order_unit_id;
		static Unit* unit;
		__asm {
			MOV order_unit_id, ECX
			MOV unit, EDX
			PUSHAD
		}
		result = hooks::get_generic_unit_idle_order(order_unit_id, unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) input_game_right_mouse_click_wrapper() { // 004564E0
		static DialogEvent* dlg_event;
		__asm {
			MOV dlg_event, ECX
			PUSHAD
		}
		hooks::input_game_right_mouse_click(dlg_event);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_right_click_action_order_wrapper() { // 004E5EA0
		static char result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::get_right_click_action_order(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) can_do_targeted_order_wrapper() { // 0049A410
		static u32 result;
		static Unit* unit;
		static u8 order_id;
		__asm {
			MOV order_id, DL
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::can_do_targeted_order(unit, order_id);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}

	void __declspec(naked) unit_can_move_wrapper() { // 00455660
		static s32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_can_move(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) drone_build_execute_wrapper() { // 0045D2E0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::drone_build_execute(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_rally_hooks() {
	//	jump_patch(set_rally_position_wrapper,				0x00466910, 2);
	//	jump_patch(set_rally_unit_wrapper,					0x00466B40, 1);

	//	jump_patch(set_rally_position_wrapper_jump1,		0x00456256, 19);	// 24 - 5 of jump
	//	jump_patch(set_rally_position_wrapper_jump2,		0x0049ACEF, 19);	// 24 - 5 of jump

	//	jump_patch(set_rally_unit_wrapper_jump1,			0x0045620D, 35);	// 40 - 5 of jump
	//	jump_patch(set_rally_unit_wrapper_jump2,			0x0049ACB0, 37);	// 42 - 5 of jump

		jump_patch(order_new_unit_to_rally_wrapper,			0x00466F50, 0);

	//	jump_patch(remove_references_wrapper,				0x0049E590, 1);
	//	jump_patch(right_click_command_wrapper,				0x004560D0, 1);

	//	jump_patch(btnscond_rally_wrapper,					0x00429520, 7);

	//	jump_patch(unit_can_have_rally_wrapper,				0x0047B2E0, 3);

		// Original functonality for pack/unpack, required to be able to read/save maps and saves properly
	//	call_patch(unit_can_have_rally_original_wrapper,	0x004E3C65, 0);
	//	call_patch(unit_can_have_rally_original_wrapper,	0x004E334A, 0);

	//	jump_patch(order_acquire_generic_right_click_unit_proc_2_wrapper,	0x00455E00,	1);
	//	jump_patch(process_right_click_action_wrapper,						0x00455F20,	1);
	//	jump_patch(get_generic_unit_idle_order_wrapper,						0x00455760,	1);
	//	jump_patch(input_game_right_mouse_click_wrapper,					0x004564E0,	0);
	//	jump_patch(get_right_click_action_order_wrapper,					0x004E5EA0,	3);
	//	jump_patch(can_do_targeted_order_wrapper,							0x0049A410,	2);
	//	jump_patch(btnscond_can_move_special_case,							0x00428D40,	0);
	//	jump_patch(btnscond_can_move,										0x00428DA0,	0);
	//	jump_patch(btnscond_can_attack,										0x00428F30,	1);
	//	jump_patch(unit_can_move_wrapper,									0x00455660,	3);
	//	jump_patch(drone_build_execute_wrapper,								0x0045D2E0,	1);
	}
}
