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
	}
}
