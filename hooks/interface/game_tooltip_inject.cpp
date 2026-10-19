#include "game_tooltip.h"

#include "hook_tools.h"
#include "systems/ui/tooltip.h"

namespace {

void __declspec(naked) show_game_tooltip_wrapper() { // 00481510
    static s32 x;
    static s32 y;
    static const char* text;
    __asm {
        MOV x, EAX
        MOV EAX, [ESP + 0x04]
        MOV y, EAX
        MOV EAX, [ESP + 0x08]
        MOV text, EAX
        PUSHAD
    }
    hooks::show_game_tooltip(x, y, text);
    __asm {
        POPAD
        RETN 0x08
    }
}

void __declspec(naked) save_under_game_tooltip_wrapper() { // 00481260
    static graphics::Bitmap* dialog_bitmap;
    static s32 dialog_left;
    static s32 dialog_top;
    __asm {
        MOV dialog_top, EDX
        MOV EAX, [ESP + 0x04]
        MOV dialog_bitmap, EAX
        MOV EAX, [ESP + 0x08]
        MOV dialog_left, EAX
        PUSHAD
    }
    hooks::save_under_game_tooltip(dialog_bitmap, dialog_left, dialog_top);
    __asm {
        POPAD
        RETN 0x08
    }
}

void __declspec(naked) update_command_tooltip_wrapper() { // 004593A0
    static bw::Dialog* hovered;
    static bw::Dialog* buttonset;
    __asm {
        MOV hovered, EAX
        MOV EAX, [ESP + 0x04]
        MOV buttonset, EAX
        PUSHAD
    }
    hooks::update_command_tooltip(buttonset, hovered);
    __asm {
        POPAD
        RETN 0x04
    }
}

void __cdecl hide_game_tooltip_wrapper() { // 00481480
    ui::hide_game_tooltip();
}

} // namespace

namespace hooks {

void inject_game_tooltip_hooks() {
    jump_patch(show_game_tooltip_wrapper,       0x00481510);
    jump_patch(hide_game_tooltip_wrapper,       0x00481480);
    jump_patch(save_under_game_tooltip_wrapper, 0x00481260);
    jump_patch(stamp_game_tooltip,              0x004811E0);
    jump_patch(restore_under_game_tooltip,      0x00481160);
    jump_patch(update_command_tooltip_wrapper,  0x004593A0);
}

} // namespace hooks
