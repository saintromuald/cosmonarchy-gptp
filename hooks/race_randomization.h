#pragma once

#include "BW/enumerations.h"
#include "BW/structures.h"

#include <array>
#include <span>

class RaceRandomizer {
public:
    static RaceRandomizer& instance() {
        static RaceRandomizer randomizer;
        return randomizer;
    }

    void process_randomization(std::span<Player> players);
    std::vector<Player>& randomized_players() { return m_randomized_players; };

private:
    RaceRandomizer() = default;

private:
    std::vector<Player> m_randomized_players;
};

namespace hooks {
void randomize_races(); // 004a9a30
void inject_randomization_hooks();
};