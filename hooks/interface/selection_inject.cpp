#include "selection.h"
#include <hook_tools.h>

namespace
{
	const u32 Func_function_0046F3A0 = 0x0046F3A0;
	void __declspec(naked) unit_selection_click_wrapper() // 0046FB40
	{
		static Unit* clicked_unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x6C
			MOV EAX, 0x00596A28
			MOVZX EAX, BYTE PTR[EAX]
			PUSH EBX
			PUSH ESI
			MOV ESI, 0x00596A29
			MOVZX ESI, BYTE PTR[ESI]
			PUSH ECX
			PUSH EDX
			MOV[EBP - 0x08], EAX
			CALL Func_function_0046F3A0
			MOV[EBP - 0x04], EAX
			MOV EBX, EAX
			MOV clicked_unit, EAX
			PUSHAD
		}
		if (clicked_unit != nullptr)
		{
			hooks::unit_selection_click(clicked_unit);
		}
		__asm {
			POPAD
			POP ESI
			POP EBX
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) sort_all_units_wrapper() // 0046F040
	{
		static Unit* unit;
		static Unit** unit_list;
		static Unit** units_in_bounds;
		static u32 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV EAX, [EBP + 0x10]
			MOV unit, EAX
			MOV EAX, [EBP + 0x0C]
			MOV unit_list, EAX
			MOV EAX, [EBP + 0x08]
			MOV units_in_bounds, EAX
			PUSHAD
		}
		result = hooks::sort_all_units(unit, unit_list, units_in_bounds);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) select_multiple_units_from_unit_list_wrapper() { // 0049AEF0
		static s32 result;
		static u32 unit_count;
		static Unit** selection;
		static char play_sound;
		static u32 send_command;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit_count, EAX
			MOV EAX, [EBP + 0x0C]
			MOV selection, EAX
			MOV EAX, [EBP + 0x10]
			MOV play_sound, AL
			MOV EAX, [EBP + 0x14]
			MOV send_command, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::select_multiple_units_from_unit_list(unit_count, selection, play_sound, send_command);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) create_new_unit_selections_from_list_wrapper() { // 0049AE40
		static u32 result;
		static Unit** unit_list;
		static u32 count;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV count, EAX
			POP EAX
			MOV unit_list, EAX
			PUSHAD
		}
		result = hooks::create_new_unit_selections_from_list(unit_list, count);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_selection_hooks()
	{
		jump_patch(unit_selection_click_wrapper,					0x0046FB40, 27);
		jump_patch(sort_all_units_wrapper,							0x0046F0F0, 1);
		jump_patch(select_multiple_units_from_unit_list_wrapper,	0x0049AEF0, 2);
		jump_patch(create_new_unit_selections_from_list_wrapper,	0x0049AE40, 0);
		jump_patch(get_selected_units_in_box,						0x0046FA40, 1);
	}
}