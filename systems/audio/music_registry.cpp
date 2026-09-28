#include "music_registry.h"
#include <utility>
#include <optional>
#include <rfl/yaml.hpp>
#include <storm/sfile.h>

namespace {
Music music_def(std::vector<SFilePath> tracks, bool fade_in = false, u32 loop_start = 0) {
    return {
        .tracks = std::move(tracks),
        .fade_in = rfl::DefaultVal<bool>{fade_in},
        .loop_start = rfl::DefaultVal<u32>{loop_start},
    };
}

// Fallback if unable to load music.yml
MusicRegistry load_default_config() {
    MusicConfig config;
    config.races = {
        {RaceId::Zerg, RaceMusic{
            .in_game    = music_def({"music\\zerg1.wav", "music\\zerg2.wav", "music\\zerg3.wav"}),
            .ready_room = music_def({"music\\zrdyroom.wav"}, true),
            .victory    = music_def({"music\\zvict.wav"}, true),
            .defeat     = music_def({"music\\zdefeat.wav"}, true),
        }},
        {RaceId::Terran, RaceMusic{
            .in_game    = music_def({"music\\terran1.wav", "music\\terran2.wav", "music\\terran3.wav"}),
            .ready_room = music_def({"music\\trdyroom.wav"}, true),
            .victory    = music_def({"music\\tvict.wav"}, true),
            .defeat     = music_def({"music\\tdefeat.wav"}),
        }},
        {RaceId::Protoss, RaceMusic{
            .in_game    = music_def({"music\\protoss1.wav", "music\\protoss2.wav", "music\\protoss3.wav"}),
            .ready_room = music_def({"music\\prdyroom.wav"}, true),
            .victory    = music_def({"music\\pvict.wav"}),
            .defeat     = music_def({"music\\pdefeat.wav"}),
        }},
        {RaceId::Askosi, RaceMusic{
            .in_game = music_def({"music\\askosi1.wav", "music\\askosi2.wav"}),
            .victory = music_def({"music\\avict.wav"}),
            .defeat  = music_def({"music\\adefeat.wav"}),
        }},
    };
    config.cues["title"] = music_def({"music\\title.wav"});
    config.cues["credits"] = music_def({"music\\terran2.wav"});
    config.cues["radio_free_zerg"] = music_def(
        {"music\\RadioFreeZerg.wav", "music\\zerg1.wav", "music\\zerg2.wav", "music\\zerg3.wav"}, false, 1);
    return MusicRegistry{std::move(config)};
}
} // namespace

MusicRegistry MusicRegistry::load_config() {
    const auto file = sfile::try_load_file(MusicYamlPath, SFileScope::MpqOnly);
    if (file) {
        if (auto result = rfl::yaml::read<MusicConfig>(std::string(file.data.get(), file.size)); result) {
            return MusicRegistry{std::move(result.value())};
        }
    }
    return load_default_config();
}

// Trivial copy of Music here is acceptable for the superior ergonomics to reference_wrapper or raw pointer
std::optional<Music> MusicRegistry::find_racial(RaceId::Enum race, MusicCategory category) const {
    auto it = m_config.races.find(race);
    if (it == m_config.races.end()) {
        return {};
    }
    const RaceMusic& music = it->second;
    switch (category) {
        case MusicCategory::InGame:    return music.in_game;
        case MusicCategory::ReadyRoom: return music.ready_room;
        case MusicCategory::Victory:   return music.victory;
        case MusicCategory::Defeat:    return music.defeat;
    }
    return {};
}

std::optional<Music> MusicRegistry::find_cue(std::string_view cue_name) const {
    for (const auto& [name, music] : m_config.cues) {
        if (name == cue_name) {
            return music;
        }
    }
    return {};
}

Music MusicRegistry::get(RaceId::Enum race, MusicCategory category) const {
    if (auto def = find_racial(race, category)) {
        return std::move(*def);
    }
    if (auto def = find_racial(RaceId::Terran, category)) {
        return std::move(*def);
    }
    return get("title");
}

Music MusicRegistry::get(std::string_view cue_name) const {
    if (auto def = find_cue(cue_name)) {
        return std::move(*def);
    }
    if (auto def = find_cue("title")) {
        return std::move(*def);
    }
    return {};
}

Music MusicRegistry::for_music_id(u32 music_id) const {
    auto in_game = [&](RaceId::Enum race, u32 start) {
        Music def = get(race, MusicCategory::InGame);
        def.start_index = rfl::DefaultVal<u32>{start};
        return def;
    };

    switch (music_id) {
        case MusicId_None:      return {};

        case MusicId_Zerg1:     return in_game(RaceId::Zerg, 0);
        case MusicId_Zerg2:     return in_game(RaceId::Zerg, 1);
        case MusicId_Zerg3:     return in_game(RaceId::Zerg, 2);
        case MusicId_Terran1:   return in_game(RaceId::Terran, 0);
        case MusicId_Terran2:   return in_game(RaceId::Terran, 1);
        case MusicId_Terran3:   return in_game(RaceId::Terran, 2);
        case MusicId_Protoss1:  return in_game(RaceId::Protoss, 0);
        case MusicId_Protoss2:  return in_game(RaceId::Protoss, 1);
        case MusicId_Protoss3:  return in_game(RaceId::Protoss, 2);

        case MusicId_ZRdyRoom:  return get(RaceId::Zerg, MusicCategory::ReadyRoom);
        case MusicId_TRdyRoom:  return get(RaceId::Terran, MusicCategory::ReadyRoom);
        case MusicId_PRdyRoom:  return get(RaceId::Protoss, MusicCategory::ReadyRoom);

        case MusicId_ZDefeat:   return get(RaceId::Zerg, MusicCategory::Defeat);
        case MusicId_TDefeat:   return get(RaceId::Terran, MusicCategory::Defeat);
        case MusicId_PDefeat:   return get(RaceId::Protoss, MusicCategory::Defeat);

        case MusicId_ZVictory:
        case MusicId_ZVictory2:
            return get(RaceId::Zerg, MusicCategory::Victory);

        case MusicId_TVictory:
        case MusicId_TVictory2:
            return get(RaceId::Terran, MusicCategory::Victory);

        case MusicId_PVictory:
        case MusicId_PVictory2:
            return get(RaceId::Protoss, MusicCategory::Victory);

        case MusicId_Title:          return get("title");
        case MusicId_Terran2Credits: return get("credits");
        case MusicId_RadioFreeZerg:  return get("radio_free_zerg");
    }
    return {};
}