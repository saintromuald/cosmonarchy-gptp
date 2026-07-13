#include "custom_glues.h"
#include "hooks/snp.h"
#include "BW/game_runner.h"
#include "BW/enumerations.h"
#include "magic_enum/magic_enum.hpp"

namespace {
static std::unique_ptr<GluJoinCrownlinkContext> g_context;

static constexpr u16 timer_index = 0x2d;
static constexpr u32 timer_duration = 500;
static constexpr u32 password_max_length = 50;

static u32 __fastcall dlgproc_glu_join_crownlink_listbox(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
        case DialogUserType_Create: {
            g_context->update_listbox(dialog);
            g_context->load_selected_entry(dialog);
            if (dialog->flags & BinDlgFlags::UpdateNeeded) {
                dialog->flags &= ~BinDlgFlags::UpdateNeeded;
                helper::generic_combobox_update(dialog);
            }
            helper::set_callback_timer(dialog, timer_index, timer_duration, 0);
        } break;
        case DialogUserType_Destroy: {
            helper::clear_callback_timer(dialog, timer_index);
            return helper::generic_listbox_interact(dialog, event);
        } break;
        case DialogUserType_InitSelf: {
            if (event->param == timer_index) {
                g_context->update_listbox(dialog);
                g_context->load_selected_entry(dialog);
                g_context->update_crownlink_status();
                return 1;
            }
        } break;
        case DialogUserType_InitChildren: {
            dialog->flags |= BinDlgFlags::RemoveStyles;
        } break;
        case DialogUserType_SetSelectionIndex: {
            helper::generic_listbox_interact(dialog, event);
            g_context->load_selected_entry(dialog);
            return 1;
        } break;
        }
    }
    return helper::generic_listbox_interact(dialog, event);
}

static u32 __fastcall dlgproc_glu_join_crownlink(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_Create: {
                g_context->on_create(dialog, event);
            } break;
            case DialogUserType_Activate: {
                return g_context->on_activate(dialog, event);
            } break;
            case DialogUserType_InitChildren: {
                g_context->on_init_children(dialog, event);
            } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}
} // namespace

void GluJoinCrownlinkContext::update_password(const char* password) {
    auto& provider_manager = snp::ProviderManager::instance();
    provider_manager.set_lobby_password(password);
    std::string label_value = "No Password Set";
    if (*password) {
        label_value = std::format("Password:{} {}", (char)LobbyTextColor::Red, password);
    }
    update_label(GluJoinControls::LobbyPasswordLabel, label_value.c_str());
}

void GluJoinCrownlinkContext::update_crownlink_status() {
    std::string label = std::format("{}Disconnected", (char)LobbyTextColor::Grey);
    auto& provider_manager = snp::ProviderManager::instance();
    CrowServeState status{};

    if (provider_manager.get_crownlink_status(status)) {
        switch (status) {
            case CrowServeState::Disconnected: {
                label = std::format("{}Disconnected", (char)LobbyTextColor::Grey);
            } break;
            case CrowServeState::Connecting: {
                label = std::format("{}Connecting{}", (char)LobbyTextColor::Red, std::string(m_ellipsis_counter % 4, '.'));
                m_ellipsis_counter++;
            } break;
            case CrowServeState::Ready: {
                label = std::format("{}Connected", (char)LobbyTextColor::Green);
            } break;
        }
    }
    update_label(GluJoinControls::CrownLinkStatus, label.c_str());
}

void GluJoinCrownlinkContext::update_label(GluJoinControls control, const char* text) {
    m_labels.update(get_dialog(), control, text);
}

void GluJoinCrownlinkContext::clear_lobby_labels() {
    update_label(GluJoinControls::GameTitle, "");
    update_label(GluJoinControls::MapName, "");
    update_label(GluJoinControls::GameType, "");
    update_label(GluJoinControls::Replay, "");
    update_label(GluJoinControls::SubType, "");
    update_label(GluJoinControls::MapSize, "");
}

void GluJoinCrownlinkContext::update_lobby_labels(GameData& game_data) {
    update_label(GluJoinControls::GameTitle, game_data.host_name);
    update_label(GluJoinControls::MapName, game_data.map_title);
    update_label(GluJoinControls::GameType, magic_enum::enum_name((GameType::Enum)game_data.game_type).data());
    update_label(GluJoinControls::Replay, game_data.is_replay ? "Replay" : "");
    update_label(GluJoinControls::SubType, "");
    auto map_dimensions = std::format("{}x{}", game_data.map_width_tiles, game_data.map_height_tiles);
    update_label(GluJoinControls::MapSize, map_dimensions.c_str());
}

void GluJoinCrownlinkContext::load_selected_entry(BinDlg* dialog) {
    auto selected_adfile = selected_adfile_index(dialog);

    auto& game_runner = GameRunner::instance();
    if (selected_adfile && game_runner.load_adfile(selected_adfile)) {
        auto& game_data = game_runner.game_data();
        m_gamedata = game_data;
        update_lobby_labels(game_data);
    } else {
        m_gamedata = {};
        clear_lobby_labels();
    }
}

u32 GluJoinCrownlinkContext::selected_adfile_index(BinDlg* dialog) const {

    if (dialog->ComboBox0x46.item_count &&
        dialog->ComboBox0x46.selection_index != -1 &&
        dialog->ComboBox0x46.selection_index < m_adfile_indexes.size()) {
        return m_adfile_indexes[dialog->ComboBox0x46.selection_index];
    }
    return 0;
}

std::string GluJoinCrownlinkContext::p2p_status_string(AdFileEx& adfile) const {
    switch (adfile.peer_state) {
        case P2PState::Disconnected: return std::format("{}[-] ", (char)LobbyTextColor::Grey);
        case P2PState::Connecting: return std::format("{}[...] ", (char)LobbyTextColor::Grey);
        case P2PState::Failed: return std::format("{}[!!] ", (char)LobbyTextColor::Grey);
        case P2PState::Relay: return "[R] ";
        case P2PState::Radmin: return "[Radmin] ";
        case P2PState::BadVersion: return std::format("{}[!Ver]{} ", (char)LobbyTextColor::Red, (char)LobbyTextColor::Revert);
    }
    return {};
}

std::string GluJoinCrownlinkContext::format_listbox_line(AdFileEx& adfile) const {
    // TODO version check
    std::string output;
    output += std::format("{}{}/{}{} ",
        (char)LobbyTextColor::White,
        adfile.extra_bytes.human_players,
        adfile.extra_bytes.max_players,
        (char)LobbyTextColor::Revert);
    output += p2p_status_string(adfile);
    bw::StatString stats(adfile.game_description);
    auto& data = stats.game_data();
    output += data.host_name;
    output += " ";
    output += (char)LobbyTextColor::Blue;
    output += data.map_title;
    return output;
}

void GluJoinCrownlinkContext::update_listbox(BinDlg* dialog) {
    auto& game_runner = GameRunner::instance();
    auto selected_adfile = selected_adfile_index(dialog);

    dialog->flags |= BinDlgFlags::UpdateNeeded;
    helper::combobox_clear_items(dialog);

    game_runner.update_adfiles();
    m_adfile_indexes.clear();
    m_listbox_entries.clear();
    for (auto& adfile : game_runner.adfiles()) {
        if (adfile.game_state == 12) continue; // Game in progress. TODO: Create an enum
        m_adfile_indexes.push_back(adfile.game_index);
        m_listbox_entries.push_back(format_listbox_line(adfile));
        auto new_index = helper::combobox_add_item(dialog, 0, m_listbox_entries.back().c_str());
    }
    
    if (dialog->flags & BinDlgFlags::UpdateNeeded) {
        dialog->flags &= ~BinDlgFlags::UpdateNeeded;
        helper::generic_combobox_update(dialog);
    }

    if (m_adfile_indexes.size()) {
        enable_dialogs_with_index(get_dialog(), GluJoinControls::OK);
    } else {
        disable_dialogs_with_index(get_dialog(), GluJoinControls::OK);
    }

    DialogEvent event{};
    event.param = -1;
    if (selected_adfile) {
        for (auto [list_index, adfile_index] : std::views::enumerate(m_adfile_indexes)) {
            if (adfile_index == selected_adfile) event.param = list_index;
        }
    } else if (dialog->ComboBox0x46.item_count) {
        event.param = 0;
    }
    event.user = DialogUserType_SetSelectionIndex;
    event.number_order = DialogEventType_Dialog;
    dialog->interaction_function(dialog, &event);
    helper::generic_combobox_selection_update(dialog, event.param);
}

void GluJoinCrownlinkContext::on_create(BinDlg* dialog, DialogEvent* event) {
    bindlg_load_background_palette(dialog);
    disable_dialogs_with_index(dialog, GluJoinControls::OK);
    auto& provider_manager = snp::ProviderManager::instance();
    std::string password;
    provider_manager.get_lobby_password(password);
    update_password(password.c_str());
}

u32 GluJoinCrownlinkContext::on_activate(BinDlg* dialog, DialogEvent* event) {
    switch (static_cast<GluJoinControls>(last_control_index)) {
        case GluJoinControls::OK: {
            disable_dialogs_with_index(dialog, GluJoinControls::OK);
            if (helper::join_game(&m_gamedata)) {
                helper::dialog_swish_out(dialog);
            } else {
                enable_dialogs_with_index(dialog, GluJoinControls::OK);
            }
            return 1;
        } break;
        case GluJoinControls::LobbyPasswordSet: {
            auto disallowed_chars = g_string_empty;
            char password[password_max_length + 5]{};
            if (helper::glu_modal_string_prompt("Enter a new password", password, password_max_length, disallowed_chars) && password[0]) {
                update_password(password);
            }
            return 1;
        } break;
        case GluJoinControls::LobbyPasswordClear: {
            update_password("");
            return 1;
        } break;
    }
    return helper::generic_dialog_interact(dialog, event);
}

void GluJoinCrownlinkContext::on_init_children(BinDlg* dialog, DialogEvent* event) {
    static const std::map<GluJoinControls, BinDlgProc> interaction_functions{
        {GluJoinControls::GameListBox, dlgproc_glu_join_crownlink_listbox}, // join_listbox
        {GluJoinControls::GameTitle, (BinDlgProc)0x004b7ef0},   // Very generic but sets flag 0x40000000. Seems to make the text white.
        {GluJoinControls::GameType, (BinDlgProc)0x004b7ef0},    //
        {GluJoinControls::SubType, (BinDlgProc)0x004b7ef0},     //
        {GluJoinControls::MapName, (BinDlgProc)0x004b7ef0},     //
        {GluJoinControls::MapSize, (BinDlgProc)0x004b7ef0},     //
        {GluJoinControls::GameSpeed, (BinDlgProc)0x004b7ef0},   //
    };
    bindlg_load_interaction_functions(dialog, interaction_functions);
}

void glu_join_crownlink() {
    g_gl_glues_mode = menu_with_theme(GluTheme::PalNl);
    g_glue_background_id = -1;

    if (!g_music_playing) helper::dialog_music_fade(MusicId::MusicId_Title);

    g_is_in_replay = false;
    if (scenario_chk) {
        SMemFree(scenario_chk, __FILE__, __LINE__, 0);
    }

    g_context = std::make_unique<GluJoinCrownlinkContext>();
    bindlg_load_background(g_context->get_dialog(), "glue\\join\\backgnd_cnlk.pcx");
    auto result = helper::glu_load_bin_dialog(g_context->get_dialog(), dlgproc_glu_join_crownlink);

    switch (static_cast<GluJoinControls>(result)) {
        case GluJoinControls::OK: {
            g_gl_glues_mode = MenuId::GlueChatroom;
        } break;
        case GluJoinControls::Cancel: {
            g_gl_glues_mode = MenuId::GlueLogin;
        } break;
        case GluJoinControls::Create: {
            g_gl_glues_mode = MenuId::GlueCreate;
        } break;
        case GluJoinControls::OtherProviders: {
            helper::glu_login_leave_game();
            g_gl_glues_mode = MenuId::GlueProviderSelectCrownlink;
        } break;
        default: {
            g_gl_glues_mode = MenuId::GlueMainMenu;
        }
    }
    g_glue_background_id = -1;
    change_menu();

    g_context.reset();
}