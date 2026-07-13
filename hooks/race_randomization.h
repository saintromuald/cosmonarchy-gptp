#pragma once

#include "BW/enumerations.h"
#include "BW/structures.h"

#include <array>
#include <span>
#include <string>

enum class RaceFlags : u32 {
    None = 0b00000000,
    Zerg = 0b00000001,
    Terran = 0b00000010,
    Protoss = 0b00000100,
    Askosi = 0b00001000,

    All = 0b00001111,
    Original = 0b00000111,
};

// Bitwise operators for enum class
inline RaceFlags operator|(RaceFlags a, RaceFlags b) {
    return static_cast<RaceFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline RaceFlags operator&(RaceFlags a, RaceFlags b) {
    return static_cast<RaceFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline RaceFlags operator^(RaceFlags a, RaceFlags b) {
    return static_cast<RaceFlags>(static_cast<u32>(a) ^ static_cast<u32>(b));
}

inline RaceFlags operator~(RaceFlags a) {
    return static_cast<RaceFlags>(~static_cast<u32>(a));
}

inline RaceFlags& operator|=(RaceFlags& a, RaceFlags b) {
    return a = a | b;
}

inline RaceFlags& operator&=(RaceFlags& a, RaceFlags b) {
    return a = a & b;
}

inline RaceFlags& operator^=(RaceFlags& a, RaceFlags b) {
    return a = a ^ b;
}

// Helper function to check if flag is set
inline bool has_flag(RaceFlags flags, RaceFlags flag) {
    return (flags & flag) != RaceFlags::None;
}

class RaceRandomizer {
public:
    static RaceRandomizer& instance() {
        static RaceRandomizer randomizer;
        return randomizer;
    }

    void process_randomization();
    const std::vector<Player>& randomized_players() { return m_randomized_players; };
    void set_player_race_flags(u32 index, RaceFlags race_flags);
    void set_player_race_flags(u32 index, u32 race_flags) { set_player_race_flags(index, static_cast<RaceFlags>(race_flags)); };
    RaceFlags player_race_flags(u32 index) const { return m_player_race_flags[index]; };
    void set_race_flags(RaceFlags race_flags) { m_race_flags = race_flags; };
    void set_race_flags(u32 race_flags) { m_race_flags = static_cast<RaceFlags>(race_flags); };
    RaceFlags race_flags() const { return m_race_flags; };
    void cleanup_and_reset();
    std::string& race_string(u32 index) { return m_player_race_strings[index]; }
    void update_config() const;

private:
    RaceRandomizer();
    
    void update_race_string(u32 index);

private:
    std::vector<Player> m_randomized_players;
    std::array<RaceFlags, PlayerLimit+1> m_player_race_flags;
    std::array<std::string, PlayerLimit+1> m_player_race_strings;
    RaceFlags m_race_flags;
};

u32 slot_id_to_index(u32 slot);
u32 index_to_slot_id(u32 index);

namespace hooks {
void randomize_races(); // 004a9a30
void inject_randomization_hooks();
};