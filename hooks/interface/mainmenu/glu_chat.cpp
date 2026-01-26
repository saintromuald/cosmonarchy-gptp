#include "custom_glues.h"
#include "glu_chat.h"
#include "BW/ui.h"
#include "BW/Storm.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "hooks/snp.h"
#include "magic_enum/magic_enum.hpp"

#include "hooks/interface/uifeatures/ui_features.h"

#include <windows.h>
#include <utility>

static std::unique_ptr<GluChatContext> g_chat_context;

void glu_chat_race_combobox_prep(BinDlg* dialog, RaceId::Enum selected_Race, u32 slot_id) {
    dialog->graphic |= 0x0100;
    if (!g_chat_unkown_3) {
        show_bin_dialog(dialog);
    }
    helper::enable_dialog(dialog);

    if (!(dialog->flags & BinDlgFlags::Highlighted)) {
        dialog->flags |= BinDlgFlags::Highlighted;
        helper::update_dialog(dialog);
    }

    quick_send_event(dialog, DialogUserType_SelectNext);
    dialog->flags |= BinDlgFlags::UpdateNeeded;
    dialog->interaction_function = helper::generic_combobox_interact;
    helper::combobox_clear_items(dialog);

    auto selected_index = hooks::load_races_dropdown(dialog, selected_Race, slot_id);

    if (dialog->flags & BinDlgFlags::UpdateNeeded) {
        dialog->flags &= ~BinDlgFlags::UpdateNeeded;
        helper::generic_combobox_update(dialog);
    }

    if (selected_index < dialog->ComboBox0x46.item_count || selected_index == 255) { 
        quick_send_event(dialog, DialogUserType_SetSelectionIndex, selected_index);
    }

    dialog->interaction_function = helper::chat_race_combobox_interact;
}

static void update_map_preview_visibility(BinDlg* dialog, bool show_map_preview, bool show_map_preview_label) { // 004b89f0
    if (g_minimap_draw_neutrals && g_chat_unkown_2 && g_chat_minimap_shown) {

        // Show/hide dialogs that will be covered by minimap preview
        char password[24]{};
        SNetGetGameInfo(SNetGetGameInfoType::Password, password, 24, 0);
        if (password[0]) {
            update_dialog_visibility(dialog, GluChatControls::GamePasswordLabel, !show_map_preview);
            update_dialog_visibility(dialog, GluChatControls::GamePasswordValue, !show_map_preview);
        }
        update_dialog_visibility(dialog, GluChatControls::GameTypeLabel, !show_map_preview);
        update_dialog_visibility(dialog, GluChatControls::GameModeValue, !show_map_preview);
        update_dialog_visibility(dialog, GluChatControls::GameTypeValue, !show_map_preview);
        update_dialog_visibility(dialog, GluChatControls::SubtypeOrReplayValue, !show_map_preview);
        update_dialog_visibility(dialog, GluChatControls::MapNameLabel, !show_map_preview);
        update_dialog_visibility(dialog, GluChatControls::MapNameValue, !show_map_preview);

        update_dialog_visibility(g_minimap_bin, 1, show_map_preview);

        update_dialog_visibility(dialog, GluChatControls::MapPreview, show_map_preview_label);
    }
}

static void scroll_chat(BinDlg* dialog, DialogUserType scroll_type) {
    auto listbox = get_first_child_by_index(dialog, GluChatControls::ChatHistoryListbox);
    if (!listbox) return;
    quick_send_event(listbox, scroll_type, 0);
}

// Simplified version of 004b9480. Extend if we add subtypes
static void update_game_info_labels(BinDlg* dialog) {
    if (!g_is_host) {
        hide_dialogs_with_index(dialog, GluChatControls::OkImage);
        hide_dialogs_with_index(dialog, GluChatControls::OkButton);
    }

    GameData& game_data = g_game_data;

    auto map_dimensions = std::format("{}x{}", game_data.map_width_tiles, game_data.map_height_tiles);
    g_chat_context->update_label(GluChatControls::MapSizeValue, map_dimensions.c_str());

    auto type_name = magic_enum::enum_name(GameType::Enum(game_data.game_type));
    g_chat_context->update_label(GluChatControls::GameModeValue, type_name.data());

    if (g_is_in_replay) {
        g_chat_context->update_label(GluChatControls::SubtypeOrReplayValue, "Replay");
    }

    g_chat_context->update_label(GluChatControls::GameNameValue, game_data.game_name);
    g_chat_context->update_label(GluChatControls::MapNameValue, game_data.map_title);
}

static void glu_chat_update_password_dialogs(BinDlg* dialog) {
    auto& provider_manager = snp::ProviderManager::instance();

    bool password_set = false;

    if (provider_manager.is_crownlink()) {
        std::string cl_password{};
        if (provider_manager.get_lobby_password(cl_password) && !cl_password.empty()) {
            password_set = true;
            g_chat_context->update_label(GluChatControls::GamePasswordValue, cl_password.c_str());
        }
    } else {
        char password[24]{};
        SNetGetGameInfo(SNetGetGameInfoType::Password, &password[0], sizeof(password), nullptr);
        if (password[0]) {
            password_set = true;
            g_chat_context->update_label(GluChatControls::GamePasswordValue, password);
        }
    }

    if (password_set) {
        show_dialogs_with_index(dialog, GluChatControls::GamePasswordLabel);
        show_dialogs_with_index(dialog, GluChatControls::GamePasswordValue);
        get_children_by_index(dialog, GluChatControls::GamePasswordLabel)[0]->graphic = 1;
        get_children_by_index(dialog, GluChatControls::GamePasswordValue)[0]->graphic = 1;
    } else {
        hide_dialogs_with_index(dialog, GluChatControls::GamePasswordLabel);
        hide_dialogs_with_index(dialog, GluChatControls::GamePasswordValue);
    }
}

static u32 glu_chat_create(BinDlg* dialog, DialogEvent* event) {
    update_game_info_labels(dialog);
    glu_chat_update_password_dialogs(dialog);
    g_chat_unkown_1 = 1;
    g_chat_unkown_2 = 1;
    g_chat_unkown_3 = 0;
    hide_dialogs_with_index(dialog, GluChatControls::StartingIn);
    hide_dialogs_with_index(dialog, GluChatControls::StartingInValue);
    auto& chat = g_chat_slot_entries;
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

static void map_preview_init(BinDlg* dialog) {
    g_chat_unkown_2 = 0;
    g_chat_minimap_shown = 1;
    g_minimap_draw_neutrals = true;
    
    g_minimap_bin = load_file_smemalloc<BinDlg>("rez\\minimappreview.bin");
    g_minimap_bin->flags |= BinDlgFlags::Active;
    helper::fixup_and_init_dialog_data(g_minimap_bin, helper::alloc_background_image, __FILE__, __LINE__);
    initialize_dialog(g_minimap_bin, (BinDlgProc)0x004a5900);

    g_minimap_draw_neutrals = false;
    helper::set_callback_timer(dialog, 1, 1000, (TimerCallback)0x004b9a50);
}

static u32 __fastcall dlgproc_ok_button_host_only(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog && event->user == DialogUserType_Show && !g_is_host) {
        return 1;
    }
    return helper::g_default_dialogprocs[std::to_underlying(dialog->control_type)](dialog, event);
};

static u32 glu_chat_init_children(BinDlg* dialog, DialogEvent* event) {
    bindlg_load_background(dialog);

    std::map<GluChatControls, BinDlgProc> interaction_functions{
        {GluChatControls::OkImage, dlgproc_ok_button_host_only},
        {GluChatControls::MapPreview, (BinDlgProc)0x004e1560},          // menu_generic_button_interact
        {GluChatControls::OkButton, dlgproc_ok_button_host_only},
        {GluChatControls::CancelButton, (BinDlgProc)0x004e1560},        // menu_generic_button_interact
        {GluChatControls::SendButton, (BinDlgProc)0x004e1560},          // menu_generic_button_interact
        {GluChatControls::ChatTextBox, (BinDlgProc)0x004b8ed0},
        {GluChatControls::ChatHistoryListbox, (BinDlgProc)0x004b96a0},
        {GluChatControls::GamePasswordLabel, (BinDlgProc)0x00419190},   // generic_label_interact
        {GluChatControls::GameTypeLabel, (BinDlgProc)0x00419190},       // generic_label_interact
        {GluChatControls::GameTypeValue, (BinDlgProc)0x00419190},       // generic_label_interact
        {GluChatControls::MapNameLabel, (BinDlgProc)0x00419190},        // generic_label_interact
        {GluChatControls::GamePasswordValue, (BinDlgProc)0x004b8e70},   // chat_gamestats_label
        {GluChatControls::GameNameValue, (BinDlgProc)0x004b8e70},       // chat_gamestats_label
        {GluChatControls::GameModeValue, (BinDlgProc)0x004b8e70},       // chat_gamestats_label
        {GluChatControls::SubtypeOrReplayValue, (BinDlgProc)0x004b8e70},// chat_gamestats_label
        {GluChatControls::MapNameValue, (BinDlgProc)0x004b8e70},        // chat_gamestats_label
        {GluChatControls::MapSizeValue, (BinDlgProc)0x004b8e70},        // chat_gamestats_label
        {GluChatControls::GameSpeedValue, (BinDlgProc)0x004b8e70},      // chat_gamestats_label
        {GluChatControls::StartingIn, (BinDlgProc)0x00419190},          // generic_label_interact
        {GluChatControls::StartingInValue, (BinDlgProc)0x00419190},     // generic_label_interact
    };
    bindlg_load_interaction_functions(dialog, interaction_functions);

    g_chat_destoyed = false;
    if (!g_is_in_replay && !g_game_data.save_timestamp) {
        map_preview_init(dialog);
    } else {
        hide_dialogs_with_index(dialog, GluChatControls::MapPreview);
    }
    return helper::generic_dialog_interact(dialog, event);
};

static u32 __fastcall glu_chat_dlgproc(BinDlg* dialog, DialogEvent* event) {
    switch (event->number_order) {
        case DialogEventType_Dialog: {
            switch (event->user) {
                case DialogUserType_Activate: return helper::glu_chat_dlgproc_activate(dialog);
                case DialogUserType_Create: return glu_chat_create(dialog, event);
                case DialogUserType_Destroy: return glu_chat_destroy(dialog, event);
                case DialogUserType_InitChildren: return glu_chat_init_children(dialog, event);
            }
        } break;
        case DialogEventType_MouseMove: {
            auto map_preview = get_first_child_by_index(dialog, GluChatControls::MapPreview);
            if (!map_preview) break;
            update_map_preview_visibility(dialog, map_preview->bounds.contains_ltrb(event->current), true);
        } break;
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

    g_chat_context = std::make_unique<GluChatContext>();

    auto last_control = static_cast<GluChatControls>(helper::glu_load_bin_dialog(g_chat_bin, glu_chat_dlgproc));

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

    g_chat_context.reset();
}

void GluChatContext::update_label(GluChatControls control, const char* text) {
    m_labels.update(g_chat_bin, control, text);
}
