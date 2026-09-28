#include "glu_campaign.h"

#include "BW/game_runner.h"
#include "BW/ui.h"
#include "CampaignManager.h"
#include "systems/ui/widgets.h"
#include "systems/ui/tooltip.h"
#include "systems/ui/palette_fader.h"
#include "systems/triggers/trigger_vars.h"
#include "systems/audio/music_system.h"

#include <algorithm>
#include <format>
#include <optional>
#include <string>

constexpr size_t MaxComboboxEntries = 20;

GluCampaignList::GluCampaignList()
    : ui::Menu("", ui::PalettePreserve::FontShade | ui::PalettePreserve::WidgetColors) {
    const char hotkey_color = static_cast<char>(LobbyTextColor::YellowGreen);
    const char revert_color = static_cast<char>(LobbyTextColor::Revert);

    image(DialogBounds::from_ltrb(472, 356, 639, 446), "glue\\dynamic\\pCancel.pcx")->translucent();
    image(DialogBounds::from_ltrb(436, 336, 639, 406), "glue\\dynamic\\pOk.pcx")->translucent();
    m_cancel_button = lightup_button(DialogBounds::from_ltrb(477, 419, 608, 440),
        std::format("{}C{}ancel", hotkey_color, revert_color))->enable_hotkey('c')->set_cancel();
    m_ok_button = lightup_button(DialogBounds::from_ltrb(441, 383, 592, 404),
        std::format("{}O{}k", hotkey_color, revert_color))->enable_hotkey('o')->set_default()->disable();

    image(DialogBounds::from_xywh(0, 336, 236, 71), "glue\\dynamic\\left_upper_button.pcx")->translucent();
    image(DialogBounds::from_xywh(0, 356, 200, 91), "glue\\dynamic\\left_lower_button.pcx")->translucent();
    m_replay_button = lightup_button(DialogBounds::from_ltrb(20, 419, 200, 440),
        std::format("Watch {}R{}eplay", hotkey_color, revert_color))->enable_hotkey('r');
    m_custom_button = lightup_button(DialogBounds::from_ltrb(40, 383, 240, 404),
        std::format("Play C{}u{}stom", hotkey_color, revert_color))->enable_hotkey('u');

    image(DialogBounds::from_ltrb(0, 0, 240, 333), "glue\\dynamic\\pjoin_reverse.pcx")->translucent();
    label(DialogBounds::from_ltrb(35, 60, 230, 90), "Official Campaigns")->fontsize(ui::FontSize::Large16x);
    m_listbox = listbox(DialogBounds::from_ltrb(45, 90, 230, 325))->fontsize(ui::FontSize::Large16);

    image(DialogBounds::from_ltrb(276, 0, 640, 356), "glue\\dynamic\\plistlrg_reverse.pcx")->translucent();
    m_title = label(DialogBounds::from_xywh(300, 100, 300, 25), "")->fontsize(ui::FontSize::Large16x);
    m_description = label(DialogBounds::from_xywh(300, 135, 275, 135), "")->fontsize(ui::FontSize::Medium14);
    m_mission_count = label(DialogBounds::from_xywh(300, 270, 275, 15), "")->fontsize(ui::FontSize::Small10);
    m_author = label(DialogBounds::from_xywh(300, 290, 275, 25), "")->fontsize(ui::FontSize::Large16);
}

void GluCampaignList::load_campaign_by_index(u16 index) {
    g_campaign_manager.campaign_index = index;
    const auto selected = g_campaign_manager.selected_campaign();
    m_ok_button->disable();
    if (!selected) return;
    
    if (selected->has_error()) {
        auto description_string = std::format("{}{}", LobbyTextColor::Red, *selected->error_string);
        m_description->set_text(description_string);
        m_ok_button->disable();
        m_author->set_text("");
        m_mission_count->set_text("");
        m_title->set_text(std::format("{}Error loading campaign", LobbyTextColor::White));
        return;
    }

    auto title = std::format("{}{}", LobbyTextColor::White, selected->name);
    m_title->set_text(title);

    if (selected->description) {
        auto description = std::format("{}{}", LobbyTextColor::Blue, *selected->description);
        m_description->set_text(description);
    } else {
        m_description->set_text("");
    }

    if (selected->author) {
        auto author = std::format("Author: {}", *selected->author);
        m_author->set_text(author);
    } else {
        m_author->set_text("");
    }

    const auto completed = std::ranges::count_if(selected->missions,
        [&](const auto& mission) { return selected->is_mission_completed(mission); });
    auto mission_count = std::format("{}{}/{} missions completed", static_cast<char>(LobbyTextColor::Green), completed, selected->missions.size());
    m_mission_count->set_text(mission_count);

    std::string background = selected->background && !selected->background->empty()
        ? campaigns::resolve_asset(selected->directory.get(), *selected->background)
        : "glue\\dynamic\\backgrounds\\options.pcx";

    if (ui::BackgroundState::instance().current().empty()) {
        fade_background_in(background, ui::PalettePreserve::FontShade | ui::PalettePreserve::WidgetColors, 600);
    } else {
        change_background(background);
    }

    const auto unlocked = std::ranges::count_if(selected->missions,
        [&](const auto& mission) { return selected->is_mission_unlocked(mission); });

    if (unlocked) {
        m_ok_button->enable();
    }
    // Todo: Display "Campaign Locked" message
}

void GluCampaignList::on_init() {
    g_campaign_manager.enter();
    MusicSystem::instance().play("title");

    m_listbox->on_create([&](ui::Widget& self) {
        const auto& registry = g_campaign_manager;
        for (auto& campaign : registry.campaigns()) {
            self.add_item(campaign.name);
        }
        auto previously_selected = g_campaign_manager.selected_campaign()
            ? g_campaign_manager.campaign_index
            : 0;
        if (previously_selected) {
            quick_send_event(self.dialog()->to_bindlg(),
                DialogUserType_SetSelectionIndex,
                previously_selected);
        } else {
            load_campaign_by_index(0);
        }
    })->on_selection([&](ui::Widget&, u16 index) {
        load_campaign_by_index(index);
    });

    if (g_campaign_manager.has_transition_error()) {
        ui::modal_ok(g_campaign_manager.take_transition_error());
    }
}

void GluCampaignList::on_exit(u32 last_control) {
    if (last_control == m_ok_button->index()) {
        g_gl_glues_mode = MenuId::GluCampaignDetails;
        return;
    } else if (last_control == m_cancel_button->index()) {
        g_gl_glues_mode = MenuId::GlueLogin;
    } else if (last_control == m_replay_button->index()) {
        g_gl_glues_mode = MenuId::GlueCreateMultiplayer;
    } else if (last_control == m_custom_button->index()) {
        g_gl_glues_mode = MenuId::GlueCreate;
    }
    g_campaign_manager.exit();
}

static std::string initial_background() {
    const auto selected = g_campaign_manager.selected_campaign();
    if (selected->background && !selected->background->empty()) {
        return campaigns::resolve_asset(selected->directory.get(), *selected->background);
    }
    return "glue\\dynamic\\backgrounds\\options.pcx";
}

GluCampaignDetails::GluCampaignDetails()
    // Start with whatever background the list left on screen so the transition is
    // seamless; the mission selection below crossfades only if it differs.
    : ui::Menu(ui::BackgroundState::instance().current().empty()
        ? initial_background()
        : ui::BackgroundState::instance().current()) {
    const auto hotkey_color = static_cast<char>(LobbyTextColor::YellowGreen);
    const auto revert_color = static_cast<char>(LobbyTextColor::Revert);

    image(DialogBounds::from_ltrb(472, 356, 639, 446), "glue\\dynamic\\pCancel.pcx")->translucent();
    image(DialogBounds::from_ltrb(436, 336, 639, 406), "glue\\dynamic\\pOk.pcx")->translucent();
    m_cancel_button = lightup_button(DialogBounds::from_ltrb(477, 419, 608, 440),
        std::format("{}C{}ancel", hotkey_color, revert_color))->enable_hotkey('c')->set_cancel();
    m_ok_button = lightup_button(DialogBounds::from_ltrb(441, 383, 592, 404),
        std::format("{}O{}k", hotkey_color, revert_color))->enable_hotkey('o')->set_default();

    m_title = label(DialogBounds::from_ltrb(35, 50, 335, 75), "")->fontsize(ui::FontSize::Large16x);
    m_subtitle = label(DialogBounds::from_ltrb(50, 75, 350, 100), "")->fontsize(ui::FontSize::Medium14);

    m_description_box = group();
    m_description_box->image(DialogBounds::from_ltrb(400, 0, 639, 332), "glue\\dynamic\\pjoin.pcx")->translucent();
    m_description = m_description_box->label(DialogBounds::from_ltrb(415, 65, 600, 305), "");

    auto select_type = campaigns::MissionSelectType::ComboBox;
    if (const auto selected = g_campaign_manager.selected_campaign(); selected) {
        select_type = selected->mission_select_type.get();
        if (selected->missions.size() > MaxComboboxEntries) {
            select_type = campaigns::MissionSelectType::ListBox;
        }
    }

    if (select_type == campaigns::MissionSelectType::ComboBox) {
        image(DialogBounds::from_ltrb(0, 361, 282, 479), "glue\\dynamic\\mission_select.pcx")->translucent();
        label(DialogBounds::from_xywh(35, 390, 240, 20), "Mission Select")->fontsize(ui::FontSize::Medium14);
        m_combobox = combobox(DialogBounds::from_xywh(35, 410, 230, 30));
    } else {
        image(DialogBounds::from_ltrb(0, 283, 282, 479), "glue\\dynamic\\mission_select_listbox.pcx")->translucent();
        label(DialogBounds::from_xywh(35, 312, 240, 20), "Mission Select")->fontsize(ui::FontSize::Medium14);
        m_combobox = listbox(DialogBounds::from_xywh(35, 340, 230, 100));
    }

    m_reset_button = lightup_button(DialogBounds::from_ltrb(25, 450, 180, 479),
        std::format("{}R{}eset Progress", hotkey_color, revert_color))
        ->enable_hotkey('r')->fontsize(ui::FontSize::Medium14)
        ->tooltip("Reset all progress for this campaign");
}

void GluCampaignDetails::on_init() {
    m_combobox->on_create([](ui::Widget& self) {
        const auto selected_campaign = g_campaign_manager.selected_campaign();
        std::optional<u16> first_uncompleted_index;
        u16 last_index = 0;
        for (auto& mission : selected_campaign->missions) {
            if (!selected_campaign->is_mission_unlocked(mission)) continue;
            last_index = self.add_item(mission.name);
            if (!first_uncompleted_index && !selected_campaign->is_mission_completed(mission)) {
                first_uncompleted_index = last_index;
            }
        }
        const u16 target = first_uncompleted_index.value_or(last_index);
        quick_send_event(self.dialog()->to_bindlg(), DialogUserType_SetSelectionIndex, target);

        if (g_campaign_manager.has_transition_error()) {
            ui::modal_ok(g_campaign_manager.take_transition_error());
        }
    });
    m_combobox->on_selection([&](ui::Widget&, u16 index) {
        g_campaign_manager.mission_index = index;
        const auto selected = g_campaign_manager.selected_campaign();
        const auto& mission = selected->missions[index];

        auto title = std::format("{}{}", static_cast<char>(LobbyTextColor::YellowGreen), mission.name);
        m_title->set_text(title);

        if (mission.subtitle) {
            auto subtitle = std::format("{}{}", static_cast<char>(LobbyTextColor::Green), *mission.subtitle);
            m_subtitle->set_text(subtitle);
        } else {
            m_subtitle->set_text("");
        }

        if (mission.description) {
            m_description_box->show();
            m_description->set_text(*mission.description);
        } else {
            m_description_box->hide();
        }

        if (mission.background && !mission.background->empty()) {
            auto background = campaigns::resolve_asset(selected->directory.get(), *mission.background);
            change_background(background);
        } else {
            change_background(initial_background());
        }

        if (mission.music && !mission.music->empty()) {
            auto music = campaigns::resolve_asset(selected->directory.get(), *mission.music);
            MusicSystem::instance().play_from_path(music);
        } else {
            MusicSystem::instance().play("title");
        }
    });
    m_reset_button->on_activate([](ui::Widget&) -> ui::ActivateResult {
        if (!ui::modal_ok_cancel("Are you sure? This will permanently reset this campaign's progress")) return ui::ActivateResult::ConsumeActivation;
        auto* campaign = g_campaign_manager.selected_campaign();
        if (campaign) {
            g_trigger_vars.get_bank(campaign->bank_name()).clear();
        }
        return ui::ActivateResult::ContinueWithDefault;
    });
}

void GluCampaignDetails::on_exit(u32 last_control) {
    if (last_control == m_ok_button->index()) {
        auto campaign = g_campaign_manager.selected_campaign();
        auto selected = g_campaign_manager.selected_mission();
        auto map_path = *campaign->directory / selected->filename;
        auto& game_runner = GameRunner::instance();
        auto result = game_runner.create_singleplayer_game(map_path, GameType::Scenario, 1);
        if (result != GameRunner::Result::Success) {
            g_gl_glues_mode = MenuId::GluCampaignList;
        }
    } else if (last_control == m_cancel_button->index()) {
        g_gl_glues_mode = MenuId::GluCampaignList;
    }
}