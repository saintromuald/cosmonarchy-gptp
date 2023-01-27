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

	void __declspec(naked) create_unit_selection_wrapper() { // 0049AE40
		static Unit* unit;
		static char index;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV index, AL
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::create_unit_selection(unit, index);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) create_selection_circle_wrapper() { // 004D7070
		static Image* result;
		static Sprite* sprite;
		static u8 color;
		static u16 image_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV image_id, AX
			MOV EAX, [EBP + 0x0C]
			MOV color, AL
			POP EAX
			MOV sprite, EAX
			PUSHAD
		}
		result = hooks::create_selection_circle(sprite, color, image_id);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) is_human_player_on_same_team_wrapper() { // 0049A110
		static b32 result;
		static u32 player_id;
		__asm {
			MOV player_id, EAX
			PUSHAD
		}
		result = hooks::is_human_player_on_same_team(player_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) illegal_team_check_wrapper() { // 00485710
		static char result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::illegal_team_check(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) game_is_team_game_wrapper() { // 00484F50
		static b32 result;
		__asm {
			PUSHAD
		}
		result = hooks::game_is_team_game();
		__asm {
			POPAD
			MOV EAX, result
			RETN
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
	//	jump_patch(create_unit_selection_wrapper,					0x004E6180, 0);
	//	jump_patch(create_selection_circle_wrapper,					0x004D7070, 5);
		jump_patch(is_human_player_on_same_team_wrapper,			0x0049A110, 1);
		jump_patch(illegal_team_check_wrapper,						0x00485710, 0);
		jump_patch(game_is_team_game_wrapper,						0x00484F50, 0);
	}
}