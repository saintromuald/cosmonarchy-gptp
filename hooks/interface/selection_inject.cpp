#include "selection.h"
#include <hook_tools.h>
#include "BW\bwdata.h"

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
        static u8 result;
        static u32 unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        result = hooks::get_team_size(unit);
        __asm {
            POPAD
            MOVZX EAX, result
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

    void __declspec(naked) create_dashed_selection_main_wrapper() { // 004E65C0
        static Unit* unit;
        static s32 player;
        __asm {
            MOV player, EDI
            MOV unit, EAX
            PUSHAD
        }
        hooks::create_dashed_selection_main(unit, player);
        __asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }

    void __declspec(naked) remove_from_selections_wrapper() { // 0049A7F0
        static Unit* unit;
        static s32 result;
        __asm {
            MOV unit, EDI
            PUSHAD
        }
        result = hooks::remove_from_selections(unit);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) select_dual_birth_units_wrapper() { // 0045D040
        static u32 result;
        static Unit* unit;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV unit, EAX
            POP EAX
            PUSHAD
        }
        result = hooks::select_dual_birth_units(unit);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x04
        }
    }

    void __declspec(naked) is_selected_unit_group_enabled_wrapper() { // 0049A2C0
        static bool result;
        __asm {
            PUSHAD
        }
        result = hooks::is_selected_unit_group_enabled();
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) select_unit_initial_wrapper() { // 0049B690
        static char result;
        __asm {
            PUSHAD
        }
        result = hooks::select_unit_initial();
        __asm {
            POPAD
            MOVZX AL, result
            RETN
        }
    }

    void __declspec(naked) create_all_selection_circles_wrapper() { // 00499A60
        static char result;
        __asm {
            PUSHAD
        }
        result = hooks::create_all_selection_circles();
        __asm {
            POPAD
            MOVZX AL, result
            RETN
        }
    }

    void __declspec(naked) update_selected_unit_data_wrapper() { // 004C38B0
        __asm {
            PUSHAD
        }
        hooks::update_selected_unit_data();
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) destroy_unit_selection_wrapper() { // 004E6290
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::destroy_unit_selection(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) set_control_group_partial() { // 00496766
        static const u32 continue_address = 0x0049676D;
        static u8 slot;
        static UnitIndex index;
        static u32 byte_offset;
        __asm {
            MOV slot, AL // THIS IS WRONG AND NEEDS CORRECTION
            MOV index, EBX
            MOV byte_offset, EDX															 
            PUSHAD																			 
        }
        if (hooks::can_add_unit_to_active_selection(g_unit_table + index.index - 1, slot)) {
            control_group_array->player[0].control_group[0].unit_index[byte_offset] = index;
            __asm {
                POPAD 
                JMP continue_address
            }																				 
        }
        __asm {
            POPAD
            DEC EDI
            JMP continue_address
        }
    }

    void __declspec(naked) status_screen_button_wrapper() { // 00458220
        static BinDlg* dialog;//Originally took 2 args, but for some reason both are duplicates in the only use case of the function.
        __asm {
            PUSH	EBP
            MOV		EBP, ESP
            MOV		dialog, EDX
            PUSHAD
        }
        hooks::status_screen_button(dialog);
        __asm {
            POPAD
            POP		EBP
            RET
        }
    }

    void __declspec(naked) generate_group_wireframes_wrapper() {
        static BinDlg* dialog;
        __asm {
            PUSH	EBP
            MOV		EBP, ESP
            MOV		dialog, EAX
            //PUSHAD
        }
        hooks::generate_group_wireframes(dialog);
        __asm {
            //POPAD
            POP		EBP
            RET
        }
    }
}

namespace hooks {
    void inject_selection_hooks() {
        jump_patch(unit_selection_click_wrapper,					0x0046FB40, 27);
        jump_patch(sort_all_units_wrapper,							0x0046F0F0, 1);
        jump_patch(select_multiple_units_from_unit_list_wrapper,	0x0049AEF0, 2);
        jump_patch(create_new_unit_selections_from_list_wrapper,	0x0049AE40, 0);
        jump_patch(get_selected_units_in_box,						0x0046FA40, 1);
    	jump_patch(create_unit_selection_wrapper,					0x004E6180, 0);
    //	jump_patch(create_selection_circle_wrapper,					0x004D7070, 5);
        jump_patch(is_human_player_on_same_team_wrapper,			0x0049A110, 1);
        jump_patch(illegal_team_check_wrapper,						0x00485710, 0);
        jump_patch(game_is_team_game_wrapper,						0x00484F50, 0);
        jump_patch(create_dashed_selection_main_wrapper,			0x004E65C0, 1);
        jump_patch(remove_from_selections_wrapper,					0x0049A7F0, 1);
        jump_patch(select_dual_birth_units_wrapper,					0x0045D040, 1);
    //	jump_patch(is_selected_unit_group_enabled_wrapper,			0x0049A2C0, 1); // Causes issues with quick-target for building placement somehow
    //	jump_patch(select_unit_initial_wrapper,						0x0049B690, 1); // Not finished
    //	jump_patch(create_all_selection_circles_wrapper,			0x00499A60, 1); // Not finished
    //	jump_patch(update_selected_unit_data_wrapper,				0x004C38B0, 3); // Not working
    //	jump_patch(set_control_group_partial,						0x00496766, 2); // Unfinished -Veeq7
        jump_patch(destroy_unit_selection_wrapper,                  0x004E6290, 2);
        call_patch(status_screen_button_wrapper,					0x00458450);	//There's only 1 reference to this function, so avoid the extra jump by hooking it with a direct call to the wrapper instead.
        call_patch(generate_group_wireframes_wrapper,				0x004581CE);
    }
}