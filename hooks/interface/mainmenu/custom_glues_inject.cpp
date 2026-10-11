#include "custom_glues.h"
#include "hook_tools.h"

const u32 glu_main_switch_de433 = 0x004de433;
static void __declspec(naked) glu_main_switch_wrapper() { // 004de445
    glu_main_switch();
    __asm {
        JMP glu_main_switch_de433
    }
}

// jump 00451be7 -> 00451bed
static void __declspec(naked) chat_race_combobox_wrapper() { // 00451aa0
    static const u32 jump_address = 0x00451bed;
    static BinDlg* dialog;
    static RaceId::Enum selected_race;
    static u32 player_id;
    __asm {
        MOV dialog, EAX
        MOV selected_race, DL
        PUSHAD
        MOV EBX, DWORD PTR [ESP + 0x24]
        MOV player_id, EBX
    }
    glu_chat_race_combobox_prep(dialog, selected_race, player_id);
    __asm {
        POPAD
        JMP jump_address
    }
}

namespace hooks {
void inject_custom_glues() {
    jump_patch(glu_main_switch_wrapper, 0x004de445);
    jump_patch(chat_race_combobox_wrapper, 0x00451be7);
    // Remove Sleep(1000) from glu_login cancel
    // It's not necessary with new flow to intentionally delay snetdestroy
    nop_patch(0x004c4690, 11);
}

}