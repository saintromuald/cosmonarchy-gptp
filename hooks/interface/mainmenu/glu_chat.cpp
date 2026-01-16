#include "custom_glues.h"
#include "BW/ui.h"
#include "BW/Storm.h"
#include "BW/bwdata.h"

#include <windows.h>
#include <utility>

static void update_map_preview_visibility(BinDlg* dialog, DialogEvent* event) {
    auto map_preview = get_first_child_by_index(dialog, GluChatControls::MapPreview);
    if (!map_preview) return;
    if (map_preview->dialog_bounds.contains(event->current)) {
        helper::glu_chat_update_control_visibility(dialog, true, true);
    } else {
        helper::glu_chat_update_control_visibility(dialog, false, true);
    }
}

static void scroll_chat(BinDlg* dialog, DialogUserType scroll_type) {
    auto listbox = get_first_child_by_index(dialog, GluChatControls::ChatHistoryListbox);
    if (!listbox) return;
    quick_send_event(listbox, scroll_type, 0);
}

static u32 glu_chat_create(BinDlg* dialog, DialogEvent* event) {
    helper::glu_chat_prep_labels(dialog);
    helper::dialog_swish_in(dialog);
    helper::generic_dialog_interact(dialog, event);
    helper::glu_chat_dlgproc_create(dialog);
    return 1;
};

static u32 glu_chat_destroy(BinDlg* dialog, DialogEvent* event) {
    helper::glu_chat_dlgproc_destroy(dialog);
    helper::glu_chat_map_preview_destroy(dialog);
    return helper::generic_dialog_interact(dialog, event);
};

static u32 glu_chat_init_children(BinDlg* dialog, DialogEvent* event) {
    helper::register_menu_functions((BinDlgProc)0x0051a4a8, dialog, 336, 0);
    g_chat_destoyed = false;
    if (!g_is_in_replay && !g_game_data.save_timestamp) {
        helper::glu_chat_map_preview_init(dialog);
        
    } else {
        hide_dialogs_with_index(dialog, GluChatControls::MapPreview);
    }
    return helper::generic_dialog_interact(dialog, event);
};

static u32 glu_chat_x405(BinDlg* dialog) {
    helper::glu_chat_update_password_field(dialog);
    g_chat_unkown_1 = 1;
    g_chat_unkown_2 = 1;
    g_chat_unkown_3 = 0;
    //hide_dialogs_with_index(dialog, GluChatControls::GameNameValue); // TODO: This never gets unhidden for some reason?
    hide_dialogs_with_index(dialog, GluChatControls::GameTypeValue);
    return 1;
};

static u32 __fastcall glu_chat_dlgproc(BinDlg* dialog, DialogEvent* event) {
    switch (event->number_order) {
        case DialogEventType_Dialog: {
            switch (event->user) {
                case DialogUserType_Activate: return helper::glu_chat_dlgproc_activate(dialog);
                case DialogUserType_Create: return glu_chat_create(dialog, event);
                case DialogUserType_Destroy: return glu_chat_destroy(dialog, event);
                case DialogUserType_InitChildren: return glu_chat_init_children(dialog, event);
                case DialogUserType_x405: return glu_chat_x405(dialog);
            }
        } break;
        case DialogEventType_MouseMove: {
            update_map_preview_visibility(dialog, event);
        } // The original code falls through here but I don't think it's actually necessary
        case DialogEventType_Spacebar: {
            if (event->virtual_key == VK_HELP) SNetGetLeaguePlayerName(&g_bnet_player_name, 25);
        } break;
        case DialogEventType_MWheelUp: {
            scroll_chat(dialog, DialogUserType_ScrollUp);
            return 1;
        } break;
        case DialogEventType_MWheelDown: {
            scroll_chat(dialog, DialogUserType_ScrollDown);
            return 1;
        } break;
    }
    return helper::generic_dialog_interact(dialog, event);
}

void glu_chat() {
    g_chat_update_needed = false;
    g_chat_leaving_chat = false;
    g_chat_comboboxes_enabled = true;
    g_chat_game_is_public = false;

    g_chat_bin = load_file_smemalloc<BinDlg>("rez\\gluChat.bin");
    g_chat_bin->flags |= BinDlgFlags::Active;
    helper::fixup_and_init_dialog_data(g_chat_bin, helper::alloc_background_image, __FILE__, __LINE__);

    helper::chat_update_race_slots(g_chat_bin);

    if (GetUserDefaultLangID() == MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN) && !g_input_context) {
        g_input_context = ImmGetContext(g_main_window);
        ImmAssociateContext(g_main_window, g_input_context);
    }

    auto last_control = static_cast<GluChatControls>(helper::glu_load_bin_dialog(g_chat_bin, glu_chat_dlgproc));
    //auto last_control = static_cast<GluChatControls>(helper::glu_load_bin_dialog(g_chat_bin, (BinDlgProc)0x004b9c20));

    if (GetUserDefaultLangID() == MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN) && !g_input_context) {
        g_input_context = ImmGetContext(g_main_window);
        g_input_context = ImmAssociateContext(g_main_window, g_input_context);
    }

    if (last_control == GluChatControls::HostLeft || last_control == GluChatControls::CancelButton) {
        if (g_provider_id == 'BNET') {
            g_gl_glues_mode = MenuId::GlueBattlenet;
        } else if (g_provider_id == 'MDMX' || g_provider_id == 'MODM') {
            g_gl_glues_mode = MenuId::GlueModem;
        } else {
            g_gl_glues_mode = MenuId::GlueGameSel;
        }
        g_is_in_replay = false;
        if (g_scenario_chk) {
            SMemFree(g_scenario_chk, __FILE__, __LINE__, 0);
            g_scenario_chk = nullptr;
        }
    } else if (last_control != GluChatControls::StartGame) {
        g_gl_glues_mode == MenuId::GlueMainMenu;
    }
    change_menu();
    return;
}