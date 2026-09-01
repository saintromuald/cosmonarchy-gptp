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
        g_campaign_manager.set_transition_error(std::format("Failed to load '{}'.", next.generic_string()));
        g_gl_glues_mode = MenuId::GluCampaignList;
    }
    return true;
}

// Always returns true. Returning false delegates next-menu handling to the caller, which we don't want
b32 __stdcall continue_campaign(b32 victory) {
    g_sc_main_state = MainState::Glues;
    const auto requested = take_next_scenario();

    // Non-campaign: load next scenario if specified, otherwise back to campaign list (singleplayer menu)
    if (!g_campaign_manager.active()) {
        if (!requested.empty()) {
            return load_standalone_scenario(requested);
        }
        g_gl_glues_mode = MenuId::GluCampaignList;
        return true;
    }

    auto* campaign = g_campaign_manager.selected_campaign();
    if (!campaign) {
        g_campaign_manager.set_transition_error(std::format("Campaign manager active but no campaign loaded."));
        g_gl_glues_mode = MenuId::GluCampaignList;
        return true;
    }

    std::optional<u32> target;
    if (!requested.empty()) { // User specified a next scenario to load, so find it in the campaign
        const auto requested_path = fs::path{requested};
        for (u32 i = 0; i < campaign->missions.size(); ++i) {
            if (fs::path{campaign->missions[i].filename} != requested_path) continue;
            if (!campaign->is_mission_unlocked(campaign->missions[i])) {
                g_campaign_manager.set_transition_error(std::format("Next scenario '{}' in campaign '{}' is locked.", requested, campaign->name));
                g_gl_glues_mode = MenuId::GluCampaignDetails;
                return true;
            }
            target = i;
            break;
        }
        if (!target) {
            g_campaign_manager.set_transition_error(std::format("Next scenario '{}' was not found in campaign '{}'.", requested, campaign->name));
            g_gl_glues_mode = MenuId::GluCampaignDetails;
            return true;
        }
    } else { // User didn't specify a next scenario, fallback to default behavior
        if (victory) { // Find the next unlocked and uncompleted mission in the campaign
            for (u32 i = g_campaign_manager.mission_index + 1; i < campaign->missions.size(); ++i) {
                const auto& mission = campaign->missions[i];
                if (campaign->is_mission_unlocked(mission) && !campaign->is_mission_completed(mission)) {
                    target = i;
                    break;
                }
            }
        } else { // Replay the same mission on loss
            target = g_campaign_manager.mission_index;
        }
        if (!target) {
            // Campaign complete (or no further missions unlocked)
            // TODO: Perhaps we should have a "Campaign Complete" screen here instead of just going back to the campaign list -Jesse
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
