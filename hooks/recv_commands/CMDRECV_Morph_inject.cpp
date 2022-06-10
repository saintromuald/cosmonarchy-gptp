#include "CMDRECV_Morph.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_building_morph_wrapper() // 004C1910
	{
		static u16 wUnitType;
		__asm {
			MOV AX, [EAX + 0x01]
			MOV wUnitType, AX
			PUSHAD
		}
		hooks::cmdrecv_building_morph(wUnitType);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_unit_morph_wrapper() // 004C1990
	{
		static u16 wUnitType;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV AX, [EAX + 0x01]
			MOV wUnitType, AX
			POP EAX
			PUSHAD
		}
		hooks::cmdrecv_unit_morph(wUnitType);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) cmdrecv_train_wrapper() // 004C1C20
	{
		static char* buffer;
		__asm {
			MOV buffer, EAX
			PUSHAD
		}
		hooks::cmdrecv_train(buffer);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) cmdrecv_liftoff_wrapper() // 004C1620
	{
		static char* buffer;
		__asm {
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV buffer, EAX
			POP EAX
			PUSHAD
		}
		hooks::cmdrecv_liftoff(buffer);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) btnsact_rally_point_wrapper() // 004244A0
	{
		u32 xd;
		__asm {
			MOV xd, ECX
			PUSHAD
		}
		hooks::btnsact_rally_point(xd);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) order_acquire_generic_right_click_unit_proc_wrapper() { // 00455CC0
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
		result = hooks::order_acquire_generic_right_click_unit_proc(unit, x_pos, y_pos, target, fow_unit_id);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
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
}

namespace hooks
{
	void inject_cmdrecv_morph_hooks()
	{
		jump_patch(cmdrecv_building_morph_wrapper,							0x004C1910, 6);
		jump_patch(cmdrecv_unit_morph_wrapper,								0x004C1990, 1);
		jump_patch(cmdrecv_train_wrapper,									0x004C1C20, 6);
	//	jump_patch(cmdrecv_liftoff_wrapper,									0x004C1620, 6);
	//	jump_patch(btnsact_rally_point_wrapper,								0x004244A0, 2);
	//	jump_patch(order_acquire_generic_right_click_unit_proc_wrapper,		0x00455CC0, 2);
	//	jump_patch(order_acquire_generic_right_click_unit_proc_2_wrapper,	0x00455E00,	1);
	//	jump_patch(process_right_click_action_wrapper,						0x00455F20,	1);
	//	jump_patch(get_generic_unit_idle_order_wrapper,						0x00455760,	1);
	//	jump_patch(input_game_right_mouse_click_wrapper,					0x004564E0,	0);
	//	jump_patch(get_right_click_action_order_wrapper,					0x004E5EA0,	3);
	//	jump_patch(can_do_targeted_order_wrapper,							0x0049A410,	2);
	}
}
