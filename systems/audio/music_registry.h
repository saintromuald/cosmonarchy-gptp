#pragma once
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <utils/rfl.h>

static constexpr auto MusicYamlPath = "config\\music.yml";

struct Music {
    std::vector<SFilePath> tracks;
    rfl::DefaultVal<bool> loop = true;
    rfl::DefaultVal<bool> fade_in = false;
    rfl::DefaultVal<u32> loop_start = 0;
    rfl::DefaultVal<u32> start_index = 0;
};

struct RaceMusic {
    std::optional<Music> in_game;
    std::optional<Music> ready_room;
    std::optional<Music> victory;
    std::optional<Music> defeat;
};

// music.yml layout: `races` plus any number of top-level named cues
struct MusicConfig {
    std::map<RaceId::Enum, RaceMusic, std::less<>> races;
    rfl::ExtraFields<Music> cues;
};

enum class MusicCategory { InGame, ReadyRoom, Victory, Defeat };

class MusicRegistry {
public:
    MusicRegistry(MusicConfig config) : m_config(std::move(config)) {}

    static MusicRegistry load_config();

    Music get(RaceId::Enum race, MusicCategory category) const;
    Music get(std::string_view cue_name) const;
    Music for_music_id(u32 music_id) const;

private:
    std::optional<Music> find_racial(RaceId::Enum race, MusicCategory category) const;
    std::optional<Music> find_cue(std::string_view cue_name) const;

private:
    MusicConfig m_config;
};