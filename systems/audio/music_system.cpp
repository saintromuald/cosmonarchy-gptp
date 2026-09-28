#include "music_system.h"

#include "audio_system.h"

#include <optional>
#include <string>
#include <string_view>

MusicSystem& MusicSystem::instance() {
    static MusicSystem instance;
    return instance;
}

MusicSystem::MusicSystem() : m_registry{MusicRegistry::load_config()} {
}

void MusicSystem::play_with_fade(const Music& def, f32 fade_in, f32 fade_out, std::optional<u32> start_index) const {
    auto& audio = AudioSystem::instance();
    audio.set_music_fade_in(fade_in);
    audio.set_music_fade_out(fade_out);
    audio.play_default_music(def.tracks, MusicOptions{
        .loop = def.loop.get(),
        .loop_start = def.loop_start.get(),
        .start_index = start_index.value_or(def.start_index.get()),
    });
}

void MusicSystem::play(std::string_view cue_name) const {
    play_with_fade(m_registry.get(cue_name), MusicFadeSeconds, MusicFadeSeconds, {});
}

void MusicSystem::play(const Music& def, bool fade_in, std::optional<u32> start_index) const {
    play_with_fade(def, fade_in ? MusicFadeSeconds : 0.0f, 0.0f, start_index);
}

void MusicSystem::play_from_path(std::string_view path) const {
    play_with_fade({.tracks = {std::string{path}}}, MusicFadeSeconds, MusicFadeSeconds, {});
}