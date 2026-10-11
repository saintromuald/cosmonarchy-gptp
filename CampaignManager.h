#pragma once

#include <filesystem>
namespace fs = std::filesystem;
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <types.h>
#include <rfl.hpp>

namespace campaigns {
using CompletionVar = std::string;

struct BankVar {
    std::string bank;
    std::string var;
};

struct And;
struct Or;

using Node = rfl::Variant<CompletionVar, BankVar, rfl::Ref<And>, rfl::Ref<Or>>;

struct And { rfl::Rename<"and", std::vector<Node>> conditions; };
struct Or { rfl::Rename<"or", std::vector<Node>> conditions; };

using When = rfl::Variant<Node, std::vector<Node>>;

enum class MissionSelectType {
    ComboBox,
    ListBox,
};
} // namespace campaigns

struct Mission {
    std::string name;
    std::string filename;
    std::optional<std::string> subtitle;
    std::optional<std::string> description;
    std::optional<std::string> background;
    std::optional<std::string> music;
    std::optional<campaigns::When> when;
};

class Campaign {
public:
    std::string name;
    rfl::DefaultVal<campaigns::MissionSelectType> mission_select_type = campaigns::MissionSelectType::ComboBox;
    std::optional<std::string> description;
    std::optional<std::string> background;
    std::optional<std::string> author;
    std::vector<Mission> missions;
    rfl::Skip<fs::path> directory;
    rfl::Skip<std::string> error_string;

    bool has_error() const { return !error_string.get().empty(); }

    template <class... Args>
    void set_error(std::format_string<Args...> fmt, Args&&... args) {
        error_string = std::format(fmt, std::forward<Args>(args)...);
    }

    bool is_mission_unlocked(const Mission& mission) const;
    bool is_mission_completed(const Mission& mission) const;

    std::string bank_name() const;
    static std::string mission_key_for(std::string_view name);

private:
    bool evaluate_mission_unlock(const campaigns::Node& node) const;
    bool evaluate_mission_unlock(const campaigns::When& when) const;
};

class CampaignManager {
public:
    void enter() {
        m_active = true;
        load_campaigns();
    }

    void exit() { m_active = false; }

    bool active() const { return m_active; }

    void load_campaigns();
    const std::vector<Campaign>& campaigns() const { return m_campaigns; }

    const std::string& error_message() const { return m_error_message; }
    bool has_error() const { return !m_error_message.empty(); }

    void set_transition_error(std::string message) { m_transition_error = std::move(message); }
    std::string take_transition_error() { return std::exchange(m_transition_error, {}); }
    bool has_transition_error() const { return !m_transition_error.empty(); }

    u32 campaign_index{};
    u32 mission_index{};

    const Campaign* selected_campaign() const {
        if (!m_active) return nullptr;
        if (campaign_index >= m_campaigns.size()) return nullptr;
        return &m_campaigns[campaign_index];
    }

    const Mission* selected_mission() const {
        auto campaign = selected_campaign();
        if (!campaign) return nullptr;
        if (mission_index >= campaign->missions.size()) return nullptr;
        return &campaign->missions[mission_index];
    }

private:
    template <class... Args>
    void set_error(std::format_string<Args...> fmt, Args&&... args) {
        m_error_message = std::format(fmt, std::forward<Args>(args)...);
    }

    bool m_active = false;
    std::vector<Campaign> m_campaigns;
    std::string m_error_message;
    std::string m_transition_error;
};

inline CampaignManager g_campaign_manager;
