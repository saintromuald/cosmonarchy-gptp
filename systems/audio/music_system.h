#pragma once

#include "../types.h"
#include "music_registry.h"

#include <optional>
#include <string_view>

// TODO: Migrate music playback functions from AudioSystem here. -Jesse
class MusicSystem {
public:
    static MusicSystem& instance();

    void play(std::string_view cue_name) const;
    void play(const Music& def, bool fade_in, std::optional<u32> start_index = {}) const;
    void play_from_path(std::string_view track) const;

    const MusicRegistry& registry() const { return m_registry; }

private:
    MusicSystem();

    void play_with_fade(const Music& def, f32 fade_in, f32 fade_out, std::optional<u32> start_index) const;

private:
    MusicRegistry m_registry;
};