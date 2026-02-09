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

    if (has_flag(race_flags, RaceFlags::Terran))  races[count++] = RaceId::Terran;
    if (has_flag(race_flags, RaceFlags::Protoss)) races[count++] = RaceId::Protoss;
    if (has_flag(race_flags, RaceFlags::Zerg))    races[count++] = RaceId::Zerg;
    if (has_flag(race_flags, RaceFlags::Askosi))  races[count++] = RaceId::Askosi;

    return {races, count};
}

static std::span<const RaceId::Enum> to_span(const SelectedRacesArray& race_array) {
    return std::span<const RaceId::Enum>(race_array.first.data(), race_array.second);
}

static RaceId::Enum random_from(std::span<const RaceId::Enum> races) {
    if (!g_multiplayer_mode) {
        // Use local randoms in singleplayer so we don't get the same result repeatedly when restarting
        // Sync randoms isn't seeded at this stage in singleplayer
        // This doesn't break replays as they contain the final race assignment data
        return races[bw::rand_between_local(0, races.size() - 1)];
    }
    return races[bw::rand_between(0, races.size() - 1)];
}

void RaceRandomizer::process_randomization() {
    m_randomized_players.clear();
    for (auto& player : g_player_table) {
        if (player.race == RaceId::Random) {
            if (player.type == PlayerType::Human) {
                auto index = slot_id_to_index(player.id);
                if (index < 8 && m_player_race_flags[index] != RaceFlags::None) {
                    auto selected_races = to_race_array(m_player_race_flags[index]);
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

void RaceRandomizer::set_player_race_flags(u32 index, RaceFlags race_flags) {
    m_player_race_flags[index] = race_flags;
    update_race_string(index);
}

void RaceRandomizer::cleanup_and_reset() {
    m_player_race_flags = {};
    set_player_race_flags(0, m_race_flags);
}

RaceRandomizer::RaceRandomizer() {
    auto& races = g_config.random_races;
    if (races.size() < 2) m_race_flags = RaceFlags::All;
    for (auto& race : races) {
        m_race_flags |= magic_enum::enum_cast<RaceFlags>(race).value_or(RaceFlags::None);
    }
    cleanup_and_reset();
}

void RaceRandomizer::update_race_string(u32 index) {
    auto& flags = m_player_race_flags[index];
    auto& string = m_player_race_strings[index];

    if ((flags & RaceFlags::All) == RaceFlags::All) {
        string = "Random{All}";
        return;
    }

    string = "Random{";
    if (has_flag(flags, RaceFlags::Terran)) string += "T";
    if (has_flag(flags, RaceFlags::Protoss)) string += "P";
    if (has_flag(flags, RaceFlags::Zerg)) string += "Z";
    if (has_flag(flags, RaceFlags::Askosi)) string += "A";
    string += "}";
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

u32 slot_id_to_index(u32 slot) {
    for (auto i = 0; i < 8; i++) {
        if (g_player_slots_by_id[i] == slot) return i;
    }
    return 8;
}

u32 index_to_slot_id(u32 index) {
    return g_player_slots_by_id[index];
}
