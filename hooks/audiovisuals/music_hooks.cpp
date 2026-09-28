#include "music_hooks.h"

#include "systems/audio/audio_system.h"
#include "systems/audio/music_system.h"

#include "BW/api.h"
#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "BW/enumerations.h"
#include "BW/rand.h"

#include <optional>
#include <string_view>

using namespace std::literals;

namespace hooks {
// Vanilla plays this immediately with no fade
void play_music(u32 music_id) {
    auto& music = MusicSystem::instance();
    music.play(music.registry().for_music_id(music_id), false);
}

// Vanilla fades in when the cue's fade flag is set
void dialog_music_fade(u32 music_id) {
    switch (music_id) {
        case MusicId_TRdyRoom: {
            if (g_current_map_name == "Sunset Mission"sv
                || g_current_map_name == "Early Retirement"sv
                || g_current_map_name == "Amber Word"sv) {
                music_id = MusicId_None; // Temporary, for mapcrap
            }
        } break;
        case MusicId_PRdyRoom: {
            if (g_current_map_name == "The Arbiter"sv) {
                music_id = MusicId_None;
            }
        } break;
        case MusicId_ZRdyRoom: {
            if (g_current_map_name == "Eye of the Needle"sv) {
                music_id = MusicId_None;
            }
        } break;
    }

    auto& music = MusicSystem::instance();
    Music def = music.registry().for_music_id(music_id);
    music.play(def, def.fade_in.get());
}

void start_game_music() { // Called from 004D9966
    auto& music = MusicSystem::instance();
    const Music& def = music.registry().get((RaceId::Enum)g_hud_index, MusicCategory::InGame);
    std::optional<u32> start;
    if (!def.tracks.empty()) {
        start = bw::rand_between_local(0, (u32)def.tracks.size() - 1);
    }
    music.play(def, false, start);
}

void start_scorescreen_music() { // Called from 004B4B79
    auto race = RaceId::Zerg;
    bool victory = true;
    if (!bw::in_replay()) {
        race = (RaceId::Enum)g_player_table[g_local_nation_id].race;
        victory = g_game_result == GameResult::Victory;
    }
    auto& music = MusicSystem::instance();
    const Music& def = music.registry().get(race, victory ? MusicCategory::Victory : MusicCategory::Defeat);
    music.play(def, true);
}

// Reimplemented to pause music when volume is set to 0 instead of killing the sound buffer
void update_sound_volumes() { // 004BC320
    if (!g_direct_sound_interface) return;

    g_sfx_volume = g_big_volume_array[(g_options_list->sfx * 99) / 100] - *g_volume_reduction;

    auto& audio = AudioSystem::instance();
    if (g_options_list->music == 0) {
        g_music_volume = DSBVOLUME_MIN;
        audio.set_music_muted(true);
    } else {
        g_music_volume = g_music_volume_table[g_options_list->music] - *g_volume_reduction;
        audio.set_music_muted(false);
    }
    g_music_playing = !audio.get_music_playback().is_paused();
}
} // namespace hooks
