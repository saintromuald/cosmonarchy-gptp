#include "console.h"
#include <hook_tools.h>

namespace
{
void __declspec(naked) setTextStr_Wrapper() { // 004263E0
    static BinDlg* a1;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        PUSH EAX
        MOV EAX, [EBP + 0x08]
        MOV a1, EAX
        POP EAX
        PUSHAD
    }
    hooks::set_text_string(a1);
    _asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 4
    }
}

const u32 continue_address = 0x0045f215;
void __declspec(naked) conover_filename_wrapper() {
    __asm {
        PUSHAD
        PUSH EAX
        CALL hooks::conover_filename
        POPAD
        PUSH EAX
        JMP continue_address
    }
}

void __stdcall load_statdata_progressbar() { // 00458050
    BinDlg* dialog;
    __asm {
        PUSHAD
        MOV dialog, EAX
    }
    hooks::load_progressbar(dialog, (BinDlgProc)helper::statdata_bar_update, g_statdata_progressbar_empty,
        g_statdata_progressbar_full, (RaceId::Enum)g_console_index);
    __asm {
        POPAD
    }
}

void __stdcall load_cmdbuttons_progressbar() { // 004596a0
    BinDlg* dialog;
    __asm {
        PUSHAD
        MOV dialog, EAX
    }
    hooks::load_progressbar(dialog, (BinDlgProc)helper::cmdbuttons_bar_update, g_cmdbuttons_progressbar_empty,
        g_cmdbuttons_progressbar_full, (RaceId::Enum)g_console_index);
    __asm {
        POPAD
    }
}
} // namespace

namespace hooks
{
void inject_console_hooks() {
    call_patch(load_race_grp_and_sounds, 0x004ef275);
    call_patch(load_console_image, 0x004c3bb2);
    call_patch(load_statfluf_mouse_regions, 0x004c3be3);
    call_patch(unload_statfluf_mouse_regions, 0x004c385c);
    jump_patch(conover_filename_wrapper, 0x0045f20a);
    call_patch(load_statdata_progressbar, 0x00458200);
    call_patch(load_cmdbuttons_progressbar, 0x004598b0);
    call_patch(load_command_buttons, 0x004c3c01);
    call_patch(load_textbox, 0x004c3c0b);
    call_patch(load_statlb, 0x004c3bed);
    call_patch(load_statres, 0x004c3bf2);
    call_patch(load_wireframe_palette, 0x004c3bfc);

    // call_patch(load_console, 0x004eedae); // AISE crash. Needs investigating

    jump_patch(setTextStr_Wrapper, 0x004263E0, 0);
}
} // namespace hooks