#include "race_randomization.h"
#include "BW/enumerations.h"
#include "BW/structures.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "BW/api.h"

#include <algorithm>
#include <utility>
#include <ranges>
#include <span>

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

static RaceId::Enum random_from(std::span<const RaceId::Enum> races) {
    return races[bw::rand_between(0, races.size() - 1)];
}

void RaceRandomizer::process_randomization(std::span<Player> players) {
    m_randomized_players.clear();
    for (auto& player : players) {
        if (player.race == RaceId::Random) {
            if (player.type == PlayerType::Human) {
                player.race = random_from(human_races);
            } else {
                // Only use original races for CPU players for now
                player.race = random_from(computer_races);
            }
            // Intentional copy since players are shuffled before game start
            m_randomized_players.push_back(player);
        }
    }
}

namespace hooks {
void randomize_races() {
    auto& randomizer = RaceRandomizer::instance();
    randomizer.process_randomization(std::span<Player>(g_player_table).first(8));
}
}
