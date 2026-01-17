#include "race_randomization.h"
#include "BW/enumerations.h"
#include "BW/structures.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "BW/api.h"
#include "config.h"

#include "magic_enum/magic_enum.hpp"

#include <algorithm>
#include <utility>
#include <ranges>
#include <span>
#include <vector>

static const u8 reroll_chance_percent = 50;

static const std::array human_races{
    RaceId::Zerg,
    RaceId::Terran,
    RaceId::Protoss,
    RaceId::Askosi,
};

static const std::array computer_races{
    RaceId::Zerg,
    RaceId::Terran,
    RaceId::Protoss,
};

using SelectedRacesArray = std::pair<std::array<RaceId::Enum, RaceId::FullCount>, u32>;
static SelectedRacesArray to_race_array(RaceFlags race_flags) {
    std::array<RaceId::Enum, RaceId::FullCount> races{};
    u32 count = 0;

    if (has_flag(race_flags, RaceFlags::Zerg))    races[count++] = RaceId::Zerg;
    if (has_flag(race_flags, RaceFlags::Terran))  races[count++] = RaceId::Terran;
    if (has_flag(race_flags, RaceFlags::Protoss)) races[count++] = RaceId::Protoss;
    if (has_flag(race_flags, RaceFlags::Askosi))  races[count++] = RaceId::Askosi;

    return {races, count};
}

static std::span<const RaceId::Enum> to_span(const SelectedRacesArray& race_array) {
    return std::span<const RaceId::Enum>(race_array.first.data(), race_array.second);
}

static RaceId::Enum random_from(std::span<const RaceId::Enum> races) {
    return races[bw::rand_between(0, races.size() - 1)];
}

void RaceRandomizer::process_randomization() {
    m_randomized_players.clear();
    for (auto& player : g_player_table) {
        if (player.race == RaceId::Random) {
            if (player.type == PlayerType::Human) {
                if (m_player_race_flags[player.id] != RaceFlags::None) {
                    auto selected_races = to_race_array(m_player_race_flags[player.id]);
                    player.race = random_from(to_span(selected_races));
                } else {
                    player.race = random_from(human_races);
                }
            } else {
                // Only use original races for CPU players for now
                player.race = random_from(computer_races);
            }
            // Intentional copy since players are shuffled before game start
            m_randomized_players.push_back(player);
        }
    }
    cleanup_and_reset();
}

void RaceRandomizer::cleanup_and_reset() {
    m_player_race_flags = {};
    m_player_race_flags[0] = m_race_flags;
}

RaceRandomizer::RaceRandomizer() {
    auto& races = g_config.random_races;
    if (races.size() < 2) m_race_flags = RaceFlags::All;
    for (auto& race : races) {
        m_race_flags |= magic_enum::enum_cast<RaceFlags>(race).value_or(RaceFlags::None);
    }
    cleanup_and_reset();
}

void RaceRandomizer::save_config() {
    g_config.random_races.clear();

    if (m_race_flags == RaceFlags::All) {
        g_config.random_races.push_back("All");
        return;
    }
    if (m_race_flags == RaceFlags::Original) {
        g_config.random_races.push_back("Original");
        return;
    }
    if (has_flag(m_race_flags, RaceFlags::Zerg)) g_config.random_races.push_back("Zerg");
    if (has_flag(m_race_flags, RaceFlags::Terran)) g_config.random_races.push_back("Terran");
    if (has_flag(m_race_flags, RaceFlags::Protoss)) g_config.random_races.push_back("Protoss");
    if (has_flag(m_race_flags, RaceFlags::Askosi)) g_config.random_races.push_back("Askosi");

    save_config();
}

namespace hooks {
void randomize_races() {
    auto& randomizer = RaceRandomizer::instance();
    randomizer.process_randomization();
}
}