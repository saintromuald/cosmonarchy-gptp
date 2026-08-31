#include "campaign.h"

#include "BW/bwdata.h"
#include "BW/enumerations.h"
#include "BW/game_runner.h"
#include "CampaignManager.h"

#include <filesystem>
namespace fs = std::filesystem;
#include <format>
#include <optional>
#include <string>
#include <string_view>

static std::string take_next_scenario() {
    std::string value{g_next_scenario_filename.data()};
    g_next_scenario_filename[0] = 0;
    return value;
}

static bool load_standalone_scenario(std::string_view requested) {
    fs::path current{g_current_map_filename};
    fs::path next = current.parent_path() / requested;

    if (GameRunner::instance().create_singleplayer_game(next, GameType::Scenario, 1) != GameRunner::Result::Success) {
        g_gl_glues_mode = MenuId::GlueMainMenu;
    }
    return true;
}

// Always returns true. Returning false delegates next-menu handling to the caller, which we don't want
b32 __stdcall continue_campaign(b32 victory) {
    g_sc_main_state = MainState::Glues;

    if (!victory) {
        g_gl_glues_mode = g_campaign_manager.active()
            ? MenuId::GluCampaignList
            : MenuId::GlueMainMenu;
        return true;
    }

    const auto requested = take_next_scenario();
    if (!g_campaign_manager.active()) {
        if (!requested.empty()) return load_standalone_scenario(requested);
        g_gl_glues_mode = MenuId::GlueMainMenu;
        return true;
    }

    auto* campaign = g_campaign_manager.selected_campaign();
    if (!campaign) {
        g_gl_glues_mode = MenuId::GlueMainMenu;
        return true;
    }

    std::optional<u32> target;
    if (!requested.empty()) {
        const auto requested_filename = fs::path{requested}.filename();
        for (u32 i = 0; i < campaign->missions.size(); ++i) {
            if (fs::path{campaign->missions[i].filename}.filename() != requested_filename) continue;
            if (!campaign->is_mission_unlocked(campaign->missions[i])) {
                g_campaign_manager.set_transition_error(std::format("Next scenario '{}' is locked.", requested));
                g_gl_glues_mode = MenuId::GluCampaignList;
                return true;
            }
            target = i;
            break;
        }
        if (!target) {
            g_campaign_manager.set_transition_error(std::format("Next scenario '{}' was not found in campaign '{}'.", requested, campaign->name));
            g_gl_glues_mode = MenuId::GluCampaignList;
            return true;
        }
    } else {
        for (u32 i = g_campaign_manager.mission_index + 1; i < campaign->missions.size(); ++i) {
            const auto& mission = campaign->missions[i];
            if (campaign->is_mission_unlocked(mission) && !campaign->is_mission_completed(mission)) {
                target = i;
                break;
            }
        }
        if (!target) {
            // Campaign complete (or nothing left to unlock)
            g_gl_glues_mode = MenuId::GluCampaignList;
            return true;
        }
    }

    g_campaign_manager.mission_index = *target;
    const auto& mission = campaign->missions[*target];
    auto map_path = *campaign->directory / mission.filename;

    if (GameRunner::instance().create_singleplayer_game(map_path, GameType::Scenario, 1) != GameRunner::Result::Success) {
        g_campaign_manager.set_transition_error(std::format("Failed to load '{}'.", mission.name));
        g_gl_glues_mode = MenuId::GluCampaignList;
    }
    return true;
}
