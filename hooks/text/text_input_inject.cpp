#include "text_input.h"
#include "hook_tools.h"
#include "BW/bwdata.h"
#include "BW/structures.h"


namespace {

TranslateMessageProc g_saved_translate_message = nullptr;

} // namespace


namespace {

void __declspec(naked) save_name_insert_char_wrapper() { // 0x004DF740
    static DialogEvent* event;
    static bw::Dialog* dlg;
    __asm {
        MOV EAX, [ESP + 4]
        MOV event, EAX
        MOV dlg, EDI
        PUSHAD
    }
    hooks::save_name_insert_char_impl(*event, *dlg);
    __asm {
        POPAD
        RET 4
    }
}

void __declspec(naked) save_game_insert_char_wrapper() { // 0x00461C20
    static DialogEvent* event;
    static bw::Dialog* dlg;
    __asm {
        MOV event, EAX
        MOV dlg, EDI
        PUSHAD
    }
    hooks::save_game_insert_char_impl(*event, *dlg);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) click_cursor_pos_update_wrapper() { // 0x004164D0
    static bw::Dialog* dlg;
    static DialogEvent* evt;
    __asm {
        MOV dlg, ESI
        MOV evt, EDX
        PUSHAD
    }
    hooks::click_cursor_pos_update(dlg, evt);
    __asm {
        POPAD
        RET
    }
}

void __declspec(naked) draw_textbox_content_wrapper() { // 0x004167F0
    static bw::Dialog* dlg;
    __asm {
        MOV dlg, EDI
        PUSHAD
    }
    hooks::draw_textbox_content(dlg);
    __asm {
        POPAD
        RET
    }
}

} // namespace

namespace hooks {

void inject_text_input_hooks() {
    // cnc-ddraw leaves TranslateMessage untouched when re-patching the IAT in hook=4 mode
    g_saved_translate_message = g_translate_message_iat;
    memory_patch(0x004FE2C8, u32(TextInput::translate_message));

    jump_patch(textbox_interact, 0x00416980);
    jump_patch(save_name_insert_char_wrapper, 0x004DF740);
    jump_patch(save_game_insert_char_wrapper, 0x00461C20);
    jump_patch(click_cursor_pos_update_wrapper, 0x004164D0);
    jump_patch(draw_textbox_content_wrapper, 0x004167F0);

    // Replace SStrCopy(game_name, name, 0x18) in create_game_map_select_menu_proc
    call_patch(copy_game_name, 0x004A823F);
    call_patch(copy_game_name, 0x004A83C4);
    call_patch(copy_game_name, 0x004A8488);
}

void teardown_text_input_hooks() {
    if (g_translate_message_iat == TextInput::translate_message) {
        memory_patch(0x004FE2C8, u32(g_saved_translate_message));
    }
    g_saved_translate_message = nullptr;
}

} // namespace hooks