#include "ui_features.h"
#include "hook_tools.h"
#include "data_ext.h"
#include "config.h"

namespace {
    const u32 SetFontHandle = 0x0041F610;
    const u32 drawUIFeatures_jmp = 0x0048D0AE;
    void _declspec(naked) draw_ui_features_wrapper() {
        _asm {
            PUSHAD
        }
        hooks::draw_ui_features();
        _asm {
            POPAD
            CALL SetFontHandle
            JMP drawUIFeatures_jmp
        }
    }

    void _declspec(naked) queue_game_command_wrapper() {
        static u8* data;
        static u32 len;
        static u8 result;
        _asm {
            MOV data, ECX
            MOV len, EDX
            PUSHAD
        }
        hooks::queue_game_command(data, len);
        _asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }

    void __declspec(naked) select_units_wrapper() {
        static Unit** a1;
        static s32 a2;
        static s32 a3;
        static s32 a4;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            MOV a1, EBX
            MOV a2, EDI
            PUSH EAX
            MOV EAX, [EBP + 0x8]
            MOV a3, EAX
            MOV EAX, [EBP + 0xC]
            MOV a4, EAX
            POP EAX
            PUSHAD
        }
        hooks::select_units(a1, a2, a3, a4);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x8
        }
    }

    void __declspec(naked) check_hotkey_char_wrapper()
    {
        static b32 result;
        static BinDlg* dialog;
        static char* hotkey;
        __asm
        {
            MOV dialog, ECX
            MOV hotkey, EDX
            PUSHAD
        }
        result = hooks::check_hotkey_char(dialog, *hotkey);
        __asm
        {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) create_upgrade_level_tooltip_wrapper() {
        static char result;
        static char* text;
        __asm {
            MOV text, ECX
            PUSHAD
        }
        result = hooks::create_upgrade_level_tooltip(text);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) trigact_transmission_wrapper() { // 004C7890
        static b32 result;
        static TriggerAction* trigger;
        __asm {
            MOV trigger, ECX
            PUSHAD
        }
        result = hooks::trigact_transmission(trigger);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) draw_minimap_unit_box_wrapper() { // 004A4650
        static char result;
        static u32 player;
        __asm {
            MOV player, ECX
            PUSHAD
        }
        hooks::draw_minimap_unit_box(player);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) draw_all_minimap_boxes_wrapper() { // 004A4AC0
        __asm {
            PUSHAD
        }
        hooks::draw_all_minimap_boxes();
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) get_cursor_type_wrapper() { // 004D1460
        static u32 result;
        __asm {
            PUSHAD
        }
        result = hooks::get_cursor_type();
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) cmdact_right_click_minimap_wrapper() { // 004563A0
        static DialogEvent* dlg_event;
        __asm {
            MOV dlg_event, EAX
            PUSHAD
        }
        hooks::cmdact_right_click_minimap(dlg_event);
        __asm {
            POPAD
            RETN
        }
    }

    bool show_fow_unit_check(Unit* unit) {
        if (unit->status & (UnitStatus::Burrowed | UnitStatus::Cloaked | UnitStatus::CloakingForFree)) {
            if (unit->sprite && helper::get_detecting_players(unit) & *g_player_visions) {
                return true;
            }
            return false;
        }
        return true;
    }

    void __declspec(naked) show_fow_unit_check_wrapper() {
        static const u32 jmp_success = 0x004EBEA1;
        static const u32 jmp_fail = 0x004EBEC2;
        static Unit* unit;
        __asm {
            MOV unit, ESI
            PUSHAD
        }
        if (show_fow_unit_check(unit)) {
            __asm {
                POPAD
                JMP jmp_success
            }
        }
        __asm {
            POPAD
            JMP jmp_fail
        }
    }

    bool include_sprite_in_vischeck(Sprite* sprite) {
        SpriteExt* sprite_ext = get_sprite_ext(sprite);
        if (sprite_ext->is_fow_sprite) {
            return false;
        }
        return true;
    }
    
    void __declspec(naked) include_sprite_in_vischeck_wrapper() {
        static const u32 jmp_address = 0x00497C72;
        static Sprite* sprite;
        __asm {
            MOV sprite, EAX
            PUSHAD
        }
        if (include_sprite_in_vischeck(sprite)) {
            //__asm {
            //	POPAD
            //	MOV DL, [EAX+0x0C]
            //	XOR [ECX], DL
            //	JMP jmp_address
            //}
        }
        __asm {
            POPAD
            JMP jmp_address
        }
    }
    
    void create_fow_sprite_partial(Thingy* thingy, Sprite* sprite) {
        thingy->unit_id = 1; // Original functionality replaced by the jump patch

        SpriteExt* sprite_ext = get_sprite_ext(sprite);
        sprite_ext->is_fow_sprite = true;
    }

    void __declspec(naked) create_fow_sprite_partial_wrapper() {
        static const u32 jmp_address = 0x00488474;
        static Thingy* thingy;
        static Sprite* sprite;
        __asm {
            MOV thingy, ESI
            MOV sprite, EAX
            PUSHAD
        }
        create_fow_sprite_partial(thingy, sprite);
        __asm {
            POPAD
            JMP jmp_address
        }
    }

    void __declspec(naked) sprite_destructor_wrapper() {
        static Sprite* sprite;
        __asm {
            MOV sprite, EDI
            PUSHAD
        }
        sprite->free();
        __asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }

    void __declspec(naked) draw_vertical_line_wrapper() { // 0041D7D0
        static u16 iterations;
        static s16 width_offset;
        static u16 buffer_multiplier;
        static u8* result;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV width_offset, AX
            MOV EAX, [EBP + 0x0C]
            MOV buffer_multiplier, AX
            POP EAX
            MOV iterations, DX
            PUSHAD
        }
        result = hooks::draw_vertical_line(iterations, width_offset, buffer_multiplier);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            MOV EAX, result
            RETN 0x08
        }
    }

    void __declspec(naked) generic_dialog_update_wrapper() { // 004EF590
        static b32 result;
        static BinDlg* button;
        static u32 width;
        static u32 height;
        static Box16* dims;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            PUSH EAX
            MOV EAX, [EBP + 0x08]
            MOV height, EAX
            MOV EAX, [EBP + 0x0C]
            MOV dims, EAX
            POP EAX
            MOV width, EDX
            MOV button, ECX
            PUSHAD
        }
        result = hooks::generic_dialog_update(button, width, height, dims);
        __asm {
            POPAD
            MOV EAX, result
            MOV ESP, EBP
            POP EBP
            RETN 0x08
        }
    }

    void __declspec(naked) draw_stat_button_image_wrapper() { // 00456C30
        static BinDlg* dialog;
        __asm {
            MOV dialog, ESI
            PUSHAD
        }
        hooks::draw_stat_button_image(dialog);
        __asm {
            POPAD
            MOV EAX, 0
            RETN
        }
    }

    void __declspec(naked) statbtn_button_update_wrapper() { // 00458900
        static void* result;
        static BinDlg* dialog;
        __asm {
            MOV dialog, ECX
            PUSHAD
        }
        hooks::statbtn_button_update(dialog);
        __asm {
            POPAD
            MOV EAX, 0
            RETN 0x08
        }
    }

    void __declspec(naked) input_mmb_down_wrapper() { // 00484520
        static char result;
        static DialogEvent* event;
        __asm {
            MOV event, ECX
            PUSHAD
        }
        result = hooks::input_mmb_down(event);
        __asm {
            POPAD
            MOVSX EAX, result
            RETN
        }
    }

    void __declspec(naked) statport_dlg_interact_wrapper() { // 0045F290
        static bool result;
        static BinDlg* dialog;
        static DialogEvent* dlg_event;
        __asm {
            MOV dialog, ECX
            MOV dlg_event, EDX
            PUSHAD
        }
        result = hooks::statport_dlg_interact(dialog, dlg_event);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) CMDRECV_minimap_ping_wrapper() { // 004BF7A0
        static char* buffer;
        static char result;
        __asm {
            MOV buffer, EAX
            PUSHAD
        }
        result = hooks::CMDRECV_minimap_ping(buffer);
        __asm {
            POPAD
            MOVSX EAX, result
            RETN
        }
    }

    void __declspec(naked) draw_game_menu_context_wrapper() { // 004F4F70
        static BinDlg* dialog;
        static u16 result;
        __asm {
            MOV dialog, EDX
            PUSHAD
        }
        result = hooks::draw_game_menu_context(dialog);
        __asm {
            POPAD
            MOVZX EAX, result
            RETN
        }
    }

    void __declspec(naked) get_tile_image_data_wrapper() { // 0040C3B0
        static s32 offset_arg;
        static vx4 tile;
        __asm {
            MOV offset_arg, EDX
            MOV tile, ECX
            PUSHAD
        }
        hooks::get_tile_image_data(offset_arg, tile);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) get_stat_txt_string_wrapper() { // 004C36F0
        static u16 string_id;
        static char* result;
        __asm {
            MOV string_id, CX
            PUSHAD
        }
        result = hooks::get_stat_txt_string(string_id);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    void __declspec(naked) draw_show_hide_terrain_context_wrapper() { // 004A4C40
        static BinDlg* dialog;
        __asm {
            MOV dialog, ECX
            PUSHAD
        }
        hooks::draw_show_hide_terrain_context(dialog);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) draw_cancel_unit_context_wrapper() { // 00457B10
        static BinDlg* dialog;
        __asm {
            MOV dialog, EBX
            PUSHAD
        }
        hooks::draw_cancel_unit_context(dialog);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) statdata_mouseover_interact_wrapper() { // 00457CE0
        static BinDlg* main_dialog;
        static BinDlg* secondary_dialog;
        __asm {
            PUSH EBP
            MOV EBP, ESP
            MOV main_dialog, EAX
            MOV EAX, [EBP + 0x8]
            MOV secondary_dialog, EAX
            PUSHAD
        }
        hooks::statdata_mouseover_interact(main_dialog, secondary_dialog);
        __asm {
            POPAD
            MOV ESP, EBP
            POP EBP
            RETN 0x4
        }
    }

    void __declspec(naked) move_screen_to_unit_wrapper() { // 004E6020
        static Unit* unit;
        __asm {
            MOV unit, EAX
            PUSHAD
        }
        hooks::move_screen_to_unit(unit);
        __asm {
            POPAD
            RETN
        }
    }

    void __declspec(naked) get_color_id_wrapper() { // 0049B0E0
        static u8 player;
        static s32 result;
        __asm {
            MOV player, AL
            PUSHAD
        }
        result = hooks::get_color_id(player);
        __asm {
            POPAD
            MOV EAX, result
            RETN
        }
    }

    // Non-hooks \\

    const u32 replayStatBtns_27AB5 = 0x00427AB5;
    const u32 replayStatBtns_27AE9 = 0x00427AE9;
    const u32 show_dialog = 0x004186A0;
    void __declspec(naked) replayStatBtns_patch_27AB0() {
        static b32 cfg_value = g_config.show_replay_progress_bar;
        __asm {
            MOV		EAX, cfg_value
            TEST	EAX, EAX
            JE		offset hide_progress_bar
            CALL	show_dialog
            JMP		replayStatBtns_27AB5
        hide_progress_bar:
            MOV		byte ptr[EBP - 3], 0
            MOV		byte ptr[EBP - 4], 3
            MOV     EAX, 0x00501B7D
            JMP		replayStatBtns_27AE9
        }
    }

    void __declspec(naked) get_scroll_directions_partial() { // 0047F155
        static const u32 jmp_back = 0x004D12A0;
        if (g_disable_mouse_scroll) {
            __asm {
                MOV EAX, 0
                RET
            }
        }
        __asm {
            JMP jmp_back
        }
    }

    static u32 dialog_update_proc_store;
    static u32 draw_game_menu;
    void __declspec(naked) dialog_update_proc_patch_1CB64() {
        __asm {
            MOV EAX, ds: [0x006D5BF4]
            XOR EDI, EDI
            CMP EAX, EDI
            JNE offset menu_is_on				// if (GameMenuDlg) goto menu_is_on

        _return:
            LEA EAX, [EBP - 0x1C]
            RETN

        menu_is_on : // Skip ahead to the next dialog and iterate the entire list. We'll draw the game menu last.
            CMP EAX, EBX
            JNE offset _return // Sanity check, making sure GameMenuDlg is the head of the dialog list.
            TEST byte ptr[EAX + 0x18], 8
            SETNE AL
            ADD AL, AL
            MOV byte ptr draw_game_menu, AL		// (u8)draw_game_menu = (GameMenuDlg->flags & CTRL_VISIBLE) ? 2 : 0;
            JE offset _return // Don't skip dialog if it's not visible, let dialog_update_proc handle it.
            MOV EBX, [EBX]						// dlg = dlg->next;
            JMP offset _return
        }
    }

    const u32 dialog_update_proc_1CB86 = 0x0041CB86;
    const u32 dialog_update_proc_1CBFF = 0x0041CBFF;
    void __declspec(naked) dialog_update_proc_patch_1CBFA() {
        __asm {
            MOV EAX, draw_game_menu
            SUB EAX, 1
            JE offset finish_draw				// if (draw_game_menu == 1) goto finish_draw
            JNS offset do_draw					// else if (draw_game_menu == 2) goto do_draw

        _end : // Continue executing the rest of dialog_update_proc.
            MOV EAX, ds : [0x006D5E2C]
            JMP dialog_update_proc_1CBFF

        do_draw : // Prepare to iterate the game menu dialog. We set the next ptr to null so that we don't reiterate the entire list again.
            MOV draw_game_menu, EAX				// --draw_game_menu;
            MOV EBX, ds : [0x006D5BF4]			// dlg = GameMenuDlg;
            MOV EAX, [EBX]
            MOV dialog_update_proc_store, EAX	// dialog_update_proc_store = dlg->next;
            MOV dword ptr[EBX], 0				// dlg->next = nullptr;
            JMP dialog_update_proc_1CB86

        finish_draw : // Restore GameMenuDlg's next ptr.
            MOV draw_game_menu, EAX				// --draw_game_menu;
            MOV EBX, ds : [0x006D5BF4]
            MOV EAX, dialog_update_proc_store
            MOV[EBX], EAX						// GameMenuDlg->next = dialog_update_proc_store;
            JMP offset _end
        }
    }

    void __declspec(naked) StarMain_restart_replay() {
        __asm {
            MOV EAX, ds:[0x006D5A60]
            MOV ds:[0x006D0F14], EAX // *g_is_replay = *g_was_replay;
            RETN
        }
    }

    const u32 load_races_dropdown_50ba7 = 0x00450ba7;
    void __declspec(naked) load_races_dropdown_wrapper() { // 00450b40
        static u8 selected_index;
        static BinDlg* dialog;
        static RaceId::Enum selected_race;
        __asm {
            MOV dialog, ESI
            MOV CL, [EBP+8]
            MOV selected_race, CL
            PUSHAD
        }
        selected_index = hooks::load_races_dropdown(dialog, selected_race);
        __asm {
            POPAD
            MOV BL, selected_index
            JMP load_races_dropdown_50ba7
        }
    }

    void __declspec(naked) load_races_dropdown_singleplayer_wrapper() { // 004adf20
        static BinDlg* dialog;
        __asm {
            MOV dialog, EAX
            PUSHAD
        }
        hooks::load_races_dropdown_singleplayer(dialog);
        __asm {
            POPAD
            RETN
        }
    }

    const u32 race_valid_aaad6 = 0x004aaad6;
    const u32 race_invalid_aab95 = 0x004aab95;
    void __declspec(naked) is_valid_race_wrapper() { // 004aaab0
        static RaceId::Enum race;
        static bool valid;
        __asm {
            MOV race, AL
            PUSHAD
        }
        valid = hooks::is_valid_race(race);
        if (valid) {
            __asm {
                POPAD
                JMP race_valid_aaad6
            }
        }
        __asm {
            POPAD
            JMP race_invalid_aab95
        }
    }
}

namespace hooks {
    void inject_ui_features_hooks() {
        jump_patch(draw_ui_features_wrapper,				0x0048D0A9, 0);
    //	jump_patch(select_units_wrapper,					            0x0046FA00, 1);
        jump_patch(queue_game_command_wrapper,			0x00485BD0, 4);
        jump_patch(check_hotkey_char_wrapper,				0x004588C0, 3);
    //	jump_patch(create_upgrade_level_tooltip_wrapper,	            0x00459150, 3);
    	jump_patch(trigact_transmission_wrapper,			0x004C7890, 0);
        jump_patch(draw_minimap_unit_box_wrapper,			0x004A4650, 4);
        jump_patch(draw_all_minimap_boxes_wrapper,		0x004A4AC0, 0);
        jump_patch(get_cursor_type_wrapper,				0x004D1460, 0);
        jump_patch(cmdact_right_click_minimap_wrapper,	0x004563A0, 1);

        // Allow units in fow (seems to desync)
    //	jump_patch(show_fow_unit_check_wrapper,				            0x004EBE88, 2); // Allow all unit ids to create fow sprites
    //	jump_patch(include_sprite_in_vischeck_wrapper,		            0x00497C6D, 0); // Exclude fow sprites from visibility checks as they are not synced
    //	jump_patch(create_fow_sprite_partial_wrapper,		            0x0048846D, 2); // Mark sprites as fow sprites in SpriteExt

        jump_patch(sprite_destructor_wrapper,               0x00497B40, 1); // Clear SpriteExt on sprite destruction
    	jump_patch(draw_vertical_line_wrapper,			0x0041D7D0, 4);
        jump_patch(centerview_unit_group,					0x004967E0, 1);
        jump_patch(generic_dialog_update_wrapper,			0x004EF590, 1);
        jump_patch(draw_stat_button_image_wrapper,		0x00456C30, 1);
        jump_patch(statbtn_button_update_wrapper,			0x00458900, 1);
    //	jump_patch(load_statdata_bin_wrapper,				            0x00459B90, 4);
    //	jump_patch(print_stat_res_info_wrapper,				            0x004E52E0, 0); // Crashes
        jump_patch(input_mmb_down_wrapper,				0x00484520, 0);
        jump_patch(input_mmb_up,							0x004843A0, 0);
        jump_patch(input_mmb_move,						0x00484460, 1);
    //	jump_patch(statport_dlg_interact_wrapper,			            0x0045F290, 2);
        jump_patch(CMDRECV_minimap_ping_wrapper,			0x004BF7A0, 0);

    //	jump_patch(show_or_hide_minimap,					            0x004A4150, 2);
        jump_patch(draw_game_menu_context_wrapper,		   0x004F4F70, 1);
        jump_patch(get_tile_image_data_wrapper,			   0x0040C3B0, 1);
        jump_patch(get_stat_txt_string_wrapper,			   0x004C36F0, 1);
        jump_patch(draw_show_hide_terrain_context_wrapper, 0x004A4C40, 0);
        jump_patch(draw_cancel_unit_context_wrapper,	   0x00457B10, 4);
        jump_patch(statdata_mouseover_interact_wrapper,    0x00457CE0, 1);
    //  jump_patch(move_screen_to_unit_wrapper,            0x004E6020, 3); // Working, but disabled to use memory patches instead. -Pr0nogo
        jump_patch(get_color_id_wrapper,                   0x0049B0E0, 2);
        jump_patch(draw_minimap_unit_box_2,                0x004A47B0, 4);

        call_patch(get_scroll_directions_partial,		   0x0047F155, 0);
        jump_patch(replayStatBtns_patch_27AB0,			   0x00427AB0);	// Don't show replay progress bar based on config. This doesn't hide it, just never shows it. -DF

        // Make ingame menus appear above custom UI elements. -DF
        call_patch(dialog_update_proc_patch_1CB64,		0x0041CB64);
        jump_patch(dialog_update_proc_patch_1CBFA,		0x0041CBFA);

        // Passive icon colorization -- extension of color remaps. -DF
        // memory_patch(0x00459C1D, (u8)0x70); // Since it's a byte value push, this is the largest buffer size with this mempatch. Will need a jump_patch or hook to support more colors.
        // ^ No longer necessary since I've replaced load_command_buttons. -Jesse
        memory_patch(0x00456C59, &g_new_cmdbtn_palette);
        memory_patch(0x004589D5, &g_new_cmdbtn_palette);

        // Fix replay deletion on restart. -DF
        memory_patch(0x004E09DC, (u8)0xA3);//Change "MOV [g_is_restart], 1" to "MOV [g_is_restart], EAX" to make room for callpatch.
        memory_patch(0x004E09DD, (u32)0x6D0F0C);
        call_patch(StarMain_restart_replay, 0x004E09E1);

        memory_patch(0x004ca150, &load_latency_dialog);

        jump_patch(is_valid_race_wrapper, 0x004aaac2);
        jump_patch(load_races_dropdown_wrapper, 0x00450b40);
        call_patch(load_races_dropdown_singleplayer_wrapper, 0x004aed68);
    }
}