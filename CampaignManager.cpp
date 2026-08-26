#include "CampaignManager.h"

#include "BW/api.h"
#include "config.h"
#include "trigger_system/trigger_vars.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <rfl/yaml.hpp>

using namespace campaigns;

constexpr std::array OfficialCampaigns {
    "Tutorials",
    "The Lost Gods",
    "Day of Blood",
    "Sovereign Saga",
    "Butcher's Ballad",
    // The Fall (Waiting for AI voice removal)
};

// For displaying campaign read error in the UI
template <class... Args>
static Campaign error_entry_campaign(const fs::path& manifest_path,
    std::format_string<Args...> fmt, Args&&... args) {
    Campaign campaign;
    campaign.directory = manifest_path.parent_path();
    campaign.name = campaign.directory.get().filename().string();
    campaign.set_error(fmt, std::forward<Args>(args)...);
    return campaign;
}

static Campaign load_campaign(const fs::path& path) {
    std::ifstream manifest(path);
    if (!manifest.is_open()) {
        return error_entry_campaign(path, "Unable to open manifest file");
    }

    auto result = rfl::yaml::read<Campaign>(manifest);
    if (!result) {
        return error_entry_campaign(path, "{}", result.error().what());
    }
    Campaign campaign = std::move(result.value());
    campaign.directory = path.parent_path();

    if (campaign.missions.empty()) {
        campaign.set_error("Campaign has no missions");
        return campaign;
    }

    const auto campaign_dir = campaign.directory.get().lexically_normal();
    for (const auto& mission : campaign.missions) {
        const auto mission_path = (campaign_dir / mission.filename).lexically_normal();
        const auto rel = mission_path.lexically_relative(campaign_dir);
        if (rel.empty() || *rel.begin() == "..") {
            campaign.set_error("Mission '{}' filename '{}' escapes the campaign directory",
                mission.name, mission.filename);
            return campaign;
        }

        std::error_code ec;
        if (!fs::exists(mission_path, ec)) {
            campaign.set_error("Mission '{}' references missing file '{}'",
                mission.name, mission.filename);
            return campaign;
        }
    }

    return campaign;
}

bool Campaign::is_mission_unlocked(const Mission& mission) const {
    if (!mission.when) return true;
    return evaluate_mission_unlock(*mission.when);
}

bool Campaign::is_mission_completed(const Mission& mission) const {
    return g_trigger_vars.get_bank(bank_name()).get(mission_key_for(mission.name), 0);
}

bool Campaign::evaluate_mission_unlock(const Node& node) const {
    return node.visit(Overloaded{
        [&](const CompletionVar& mission_name) {
            return g_trigger_vars.get_bank(bank_name()).get(mission_key_for(mission_name), 0);
        },
        [&](const BankVar& value) {
            return g_trigger_vars.get_bank(value.bank).get(value.var, 0);
        },
        [&](const rfl::Ref<And>& value) {
            const auto& conds = value->conditions.get();
            return std::ranges::all_of(conds, [&](const Node& child) { return evaluate_mission_unlock(child); });
        },
        [&](const rfl::Ref<Or>& value) {
            const auto& conds = value->conditions.get();
            return std::ranges::any_of(conds, [&](const Node& child) { return evaluate_mission_unlock(child); });
        },
    });
}

bool Campaign::evaluate_mission_unlock(const When& when) const {
    return when.visit(Overloaded{
        [this](const Node& value) { return evaluate_mission_unlock(value); },
        // Bare list at the top level = implicit AND.
        [this](const std::vector<Node>& value) {
            return std::ranges::all_of(value, [&](const Node& child) { return evaluate_mission_unlock(child); });
        },
    });
}

static std::string to_identifier(std::string_view input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        if (c == ' ') {
            result += '_';
        } else {
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return result;
}

std::string Campaign::bank_name() const {
    return std::format("campaign_progress/{}", to_identifier(name));
}

std::string Campaign::mission_key_for(std::string_view name) {
    return std::format("_mission_completed_{}", to_identifier(name));
}

void CampaignManager::load_campaigns() {
    m_campaigns.clear();
    m_error_message.clear();

    const fs::path g_campaign_search_path = bw::get_starcraft_path()
        / "Maps" / g_config.default_map_directory / "campaign";
    std::error_code ec;
    if (!fs::exists(g_campaign_search_path, ec)) {
        set_error("Campaign directory not found: {}", g_campaign_search_path.string());
        return;
    }

    for (const auto& entry : fs::directory_iterator{g_campaign_search_path, ec}) {
        std::error_code entry_ec;
        if (!entry.is_directory(entry_ec) || entry_ec) continue;

        const auto manifest = entry.path() / "manifest.yml";
        if (!fs::exists(manifest, entry_ec)) continue;

        m_campaigns.push_back(load_campaign(manifest));
    }

    std::ranges::sort(m_campaigns, {}, [](const Campaign& campaign) {
        auto it = std::ranges::find(OfficialCampaigns, campaign.name);
        if (it != OfficialCampaigns.end()) {
            return std::make_tuple(std::distance(OfficialCampaigns.begin(), it), campaign.name);
        }
        return std::make_tuple((s32)OfficialCampaigns.size(), campaign.name);
    });
}