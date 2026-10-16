#include "custom_glues.h"
#include "hook_tools.h"
#include "glu_chat.h"

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

// For call patch wrapping vanilla loader
static void __declspec(naked) load_bin_scaled_wrapper() {
    static bw::Dialog* dialog;
    static const u32 func_address = 0x0041a080;
    __asm {
        PUSHAD
        MOV dialog, EAX
    }
    ui::scale_bin(*dialog, ui::MenuBinScale);
    __asm {
        POPAD
        JMP func_address
    }
}

// For call patch wrapping vanilla loader — glue modal boxes (savegame/ok/
// okcancel) whose roots are sub-640x480 dialogs, not fullscreen menu bins
static void __declspec(naked) load_bin_scaled_modal_wrapper() {
    static bw::Dialog* dialog;
    static const u32 func_address = 0x0041a080;
    __asm {
        PUSHAD
        MOV dialog, EAX
    }
    hooks::scale_glue_modal(dialog);
    __asm {
        POPAD
        JMP func_address
    }
}

static void __declspec(naked) chat_set_slot_label_wrapper() {
    static bw::Dialog* label;
    static const char* text;
    __asm {
        PUSH EBP
        MOV EBP, ESP
        MOV label, EAX
        MOV EAX, [EBP + 0x8]
        MOV text, EAX
        PUSHAD
    }
    hooks::chat_set_slot_label(label, text);
    __asm {
        POPAD
        MOV ESP, EBP
        POP EBP
        RETN 4
    }
}

namespace hooks {
void inject_custom_glues() {
    jump_patch(glu_main_switch_wrapper, 0x004de445);
    jump_patch(chat_race_combobox_wrapper, 0x00451be7);
    jump_patch(chat_set_slot_label_wrapper, 0x00450560, 1);
    // Remove Sleep(1000) from glu_login cancel
    // It's not necessary with new flow to intentionally delay snetdestroy
    nop_patch(0x004c4690, 11);

    // Scorescreen fixes
    // Scale untab.pcx draw location for unselected tabs
    memory_patch(0x004B385D, u8(28 * ui_scale::Factor));
    // Fix player name blit location
    memory_patch(0x004b3978, u16(0xB8 * ui_scale::Factor + u32(ui::MenuBinScale.offset.x)));

    call_patch(load_bin_scaled_wrapper, 0x004af763); // glu_create
    call_patch(load_bin_scaled_wrapper, 0x004f79cd); // glu_p_readyroom
    call_patch(load_bin_scaled_wrapper, 0x004f766d); // glu_t_readyroom
    call_patch(load_bin_scaled_wrapper, 0x004f730d); // glu_z_readyroom
    call_patch(load_bin_scaled_wrapper, 0x004b4d73); // glu_scorescreen

    // Scorescreen save replay modal and its confirm/error boxes
    call_patch(load_bin_scaled_modal_wrapper, 0x004e0118); // score_save_replay
    call_patch(load_bin_scaled_modal_wrapper, 0x004f5eb2); // okcancel.bin overwrite confirm
    call_patch(load_bin_scaled_modal_wrapper, 0x004f6022); // ok.bin error modal
}
}