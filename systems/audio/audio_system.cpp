#include "audio_system.h"
#include "audio_decoder.h"
#include "../config.h"
#include "../BW/api.h"
#include "../BW/bwdata.h"
#include "../BW/bwhelper.h"
#include "../BW/enumerations.h"
#include "../utility.h"
#include "../storm/sfile.h"
#include "../hooks/limits/sfx_extender.h"
#include "../hooks/game_hooks.h"
#include "../display/display.h"
#include <algorithm>
#include <format>
#include <utility>

static constexpr u32 MenuMusicTimerId = 3077;
static constexpr u32 MenuMusicTimerIntervalMs = 40; // ~24Hz to match next_frame's update call frequency

static bool should_check_duplicate(v2u16 existing_pos, v2u16 candidate_pos) {
    // | --------- | -------- | -------- |
    // | Candidate | Existing | Check    |
    // | --------- | -------- | -------- |
    // | Local     | Local    | If close |
    // | Global    | Local    | Never    |
    // | Global    | Global   | Always   |
    // | Local     | Global   | Always   |
    // | --------- | -------- | -------- |
    if (existing_pos) {
        if (candidate_pos) {
            // Candidate is local and existing is local, we skip the last time check if they are far away
            if (abs(existing_pos.x - candidate_pos.x) > g_display.world_width() || abs(existing_pos.y - candidate_pos.y) > g_display.world_height()) {
                return false;
            }
        } else {
            // Candidate is global and existing is local, we skip the check always
            return false;
        }
    } // Else: if existing is global, we check always
    return true;
}

bool SoundBuffer::is_blocking_play(SoundBuffer* candidate, v2u16 candidate_pos) {
    if (get_root_buffer() != candidate->get_root_buffer() || !last_play_time) {
        return false;
    }

    return should_check_duplicate(settings.get_updated_position(), candidate_pos)
        && is_playing()
        && GetTickCount() - last_play_time < 80;
}


AudioSystem::AudioSystem() {
    if (!m_direct_sound) {
        initialize_direct_sound();
    }

    initialize_sound_paths();
}

AudioSystem::~AudioSystem() {
    stop_and_clear_playing_sounds();

    for (auto& sound : m_dup_channels) {
        sound.free_buffer();
    }

    for (auto& sound : m_loaded_sounds) {
        sound.free_buffer();
    }

    m_music_playback.stream.reset(); // Pointless? unique_ptr automatically does reset on destruction -Veeq7

    if (m_direct_sound) {
        m_direct_sound->Release();
        m_direct_sound = nullptr;
    }
}

void AudioSystem::initialize_direct_sound() {
    if (SUCCEEDED(DirectSoundCreate(nullptr, &m_direct_sound, nullptr))) {
        WAVEFORMATEX wave_format = {};
        wave_format.wFormatTag = WAVE_FORMAT_PCM;
        wave_format.nChannels = 2;
        wave_format.nSamplesPerSec = 48000;
        wave_format.wBitsPerSample = 16;
        wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
        wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
        wave_format.cbSize = 0;

        if (SUCCEEDED(m_direct_sound->SetCooperativeLevel(g_main_window, DSSCL_PRIORITY))) {
            DSBUFFERDESC buffer_description = {};
            buffer_description.dwSize = sizeof(DSBUFFERDESC);
            buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

            // @Todo: what is a primary buffer needed for? seems like sounds play without too 
            IDirectSoundBuffer* primary_buffer = nullptr;
            if (SUCCEEDED(m_direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, nullptr))) {
                primary_buffer->SetFormat(&wave_format);
                primary_buffer->Release();
            }
        }
    }
}

void AudioSystem::initialize_sound_paths() {
    std::string file = mpq_load_file("arr\\sounds.txt").value_or("");

    u32 line_count = std::count(file.begin(), file.end(), '\n');
    // Add one more for the last line if file doesn't end with newline
    if (!file.empty() && file.back() != '\n') {
        ++line_count;
    }
    line_count = std::min(line_count, (u32)SoundIdLimit);

    m_sound_paths.resize(line_count);
    for (auto& path : m_sound_paths) {
        path.fill(0);
    }

    u16 sound_id = 0;
    u16 char_index = 0;
    for (const char* c = file.data(); c != file.data() + file.size(); c++) {
        if (c[0] == '\r') {
            continue;
        }
        if (c[0] == '\n') {
            char_index = 0;
            ++sound_id;
            if (sound_id >= SoundIdLimit) {
                break;
            }
            continue;
        }

        if (char_index < MAX_PATH) {
            m_sound_paths[sound_id][char_index++] = *c;
        }
    }
}

void AudioSystem::update() {
    // Clean up buffers that are done playing
    for (u32 i = 0; i < m_loaded_sounds_count; ++i) {
        SoundMainBuffer* sound = &m_loaded_sounds[i];
        if (sound->ds_buffer && GetTickCount() - sound->last_play_time >= 10 * 1000) {
            DWORD status = 0;
            sound->ds_buffer->GetStatus(&status);
            if (!(status & DSBSTATUS_PLAYING)) {
                sound->ds_buffer->Release();
                sound->ds_buffer = nullptr;
            }
        }
    }

    // Advance the music stream; drop the track when it has fully played out
    if (m_music_playback.stream) {
        m_music_playback.stream->pump();
        if (m_music_playback.stream->is_finished()) {
            m_music_playback.stream.reset();
            m_music_playback.current_music.clear();
        }
    } else {
        m_music_playback.current_music.clear();
    }

    // Handle fade out
    if (m_music_playback.target_fade == 0) {
        if (m_music_playback.fade_out) {
            m_music_playback.fade -= (1.0f / m_music_playback.fade_out) / 1secs;
        } else {
            m_music_playback.fade = 0;
        }
        if (m_music_playback.fade <= 0) {
            m_music_playback.fade = 0;
            stop_playback(m_music_playback.is_paused());
        }
    }

    // Start next music if current music stopped
    if (m_music_playback.current_music.empty() && !m_music_playback.is_paused()) {
        if (!m_music_playback.next_music.empty()) {
            m_music_playback.current_music = std::move(m_music_playback.next_music);
            m_music_playback.next_music.clear();
        } else {
            m_music_playback.current_music = get_next_music();
        }
        if (!m_music_playback.current_music.empty()) {
            m_music_playback.target_fade = 1;
            start_playback(false);
        }
    }

    // Handle fade in
    if (m_music_playback.target_fade == 1) {
        if (m_music_playback.fade_in) {
            m_music_playback.fade += (1.0f / m_music_playback.fade_in) / 1secs;
        } else {
            m_music_playback.fade = 1;
        }
        if (m_music_playback.fade >= 1) {
            m_music_playback.fade = 1;
        }
    }

    if (m_music_playback.volume_time) {
        f32 music_volume_delta = (1.0f / m_music_playback.volume_time) / 1secs;
        if (m_music_playback.volume < m_music_playback.target_volume) {
            m_music_playback.volume += music_volume_delta;
            if (m_music_playback.volume >= m_music_playback.target_volume) {
                m_music_playback.volume = m_music_playback.target_volume;
            }
        } else {
            m_music_playback.volume -= music_volume_delta;
            if (m_music_playback.volume <= m_music_playback.target_volume) {
                m_music_playback.volume = m_music_playback.target_volume;
            }
        }
    } else {
        m_music_playback.volume = m_music_playback.target_volume;
    }

    if (m_music_playback.stream) {
        m_music_playback.stream->set_volume(get_playback_volume());
    }
}

bool AudioSystem::load_ds_buffer(SoundMainBuffer* sound) {
    if (!m_direct_sound) {
        return false;
    }

    auto file = mpq_load_file(sound->path);
    if (!file) {
        bw::println("Error: File {} missing!", sound->path);
        return false;
    }

    auto decoded = decode_audio_to_pcm(std::span{(const u8*)file->data(), file->size()});
    if (!decoded) {
        bw::println("Error: File {} failed to decode!", sound->path);
        return false;
    }

    const auto size = (u32)decoded->pcm.size();
    const auto* data = (const s8*)decoded->pcm.data();

    DSBUFFERDESC buffer_description = {};
    buffer_description.dwSize = sizeof(DSBUFFERDESC);
    buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
    buffer_description.dwBufferBytes = size;
    buffer_description.lpwfxFormat = &decoded->format;

    if (!SUCCEEDED(m_direct_sound->CreateSoundBuffer(&buffer_description, &sound->ds_buffer, nullptr))) {
        return false;
    }

    u32 play_cursor = 0;
    u32 write_cursor = 0;
    if (SUCCEEDED(sound->ds_buffer->GetCurrentPosition((DWORD*)&play_cursor, (DWORD*)&write_cursor))) {
        u8* ptr1 = nullptr;
        u32 bytes1 = 0;
        u8* ptr2 = nullptr;
        u32 bytes2 = 0;

        if (SUCCEEDED(sound->ds_buffer->Lock(write_cursor, size, (void**)&ptr1, (DWORD*)&bytes1, (void**)&ptr2, (DWORD*)&bytes2, 0))) {
            if (ptr1) {
                for (u32 i = 0; i < bytes1; ++i) {
                    ptr1[i] = data[i];
                }
            }
            if (ptr2) {
                for (u32 i = 0; i < bytes2; ++i) {
                    ptr2[i] = data[i];
                }
            }

            sound->ds_buffer->Unlock(ptr1, bytes1, ptr2, bytes2);
            return true;
        }
    }

    sound->ds_buffer->Release();
    sound->ds_buffer = nullptr;
    return false;
}

SoundMainBuffer* AudioSystem::load_sound(const std::string& filename) {
    if (filename.empty()) {
        return nullptr;
    }
    if (m_loaded_sounds_count < MaxSound) {
        if (sfile::file_exists(filename.c_str())) {
            SoundMainBuffer* sound = &m_loaded_sounds[m_loaded_sounds_count++];
            auto result = std::format_to_n(sound->path, MAX_PATH - 1, "{}", filename);
            sound->path[result.size] = '\0';
            m_loaded_sounds_map[filename] = sound;
            return sound;
        }
    }
    return nullptr;
}

SoundMainBuffer* AudioSystem::get_sound(const std::string& filename) {
    if (auto it = m_loaded_sounds_map.find(filename); it != m_loaded_sounds_map.end()) {
        return it->second;
    }
    return load_sound(filename);
}

SoundMainBuffer* AudioSystem::get_sound(SoundId::Enum sound_id) {
    if (sound_id >= m_sound_paths.size()) return nullptr;
    auto filename = std::format("sound\\{}", m_sound_paths[sound_id].data());
    return get_sound(filename);
}

PlayingSound AudioSystem::play_sound(SoundMainBuffer* sound_buffer, PlaySoundSettings settings, SoundId::Enum sound_id) {
    if (!sound_buffer) {
        return {};
    }
    if (!sound_buffer->ds_buffer) {
        if (!load_ds_buffer(sound_buffer)) {
            return {};
        }
    }

    const auto request_pos = settings.get_updated_position();
    if (g_sc_main_state == MainState::Run) {
        for (auto& sound : m_playing_sounds) {
            if (auto* buffer = sound.get_buffer()) {
                if (buffer->is_blocking_play(sound_buffer, request_pos)) {
                    return {};
                }
            }
        }
    }

    if (sound_id != SoundId::None) {
        u32 flags = sfx_dat::sound_flags[sound_id];
        if (flags & DatSoundFlags_OneAtATime) {
            if (sound_buffer->is_playing()) {
                return {};
            }
        }
        if (flags & DatSoundFlags_UnitResponse) {
            if (settings.unit) {
                settings.unit_response_id = settings.unit->id;
                if (const auto pos = settings.get_updated_position()) {
                    const auto [volume, pan] = settings.calculate_volume_pan();
                    if (volume == DSBVOLUME_MIN) {
                        // atm we don't support starting banter outside of camera, for the sake of not having global collisions,
                        // potentially could filter them by distance, by mimicking the math from
                        // helper::get_sfx_volume_from_screen_distance, but between two points
                        return {};
                    }
                }
                for (const auto& playing_sound : m_playing_sounds) {
                    auto* buffer = playing_sound.get_buffer();
                    if (!buffer || buffer->settings.unit_response_id != settings.unit_response_id) {
                        continue;
                    }
                    if (buffer->is_playing()) {
                        return {};
                    }
                }
            }
        }
    }

    DWORD status = 0;
    sound_buffer->ds_buffer->GetStatus(&status);
    if (!(status & DSBSTATUS_PLAYING)) {
        const auto [volume, pan] = settings.calculate_volume_pan();
        sound_buffer->ds_buffer->SetVolume(volume);
        sound_buffer->ds_buffer->SetPan(pan);
        sound_buffer->ds_buffer->SetCurrentPosition(0);
        sound_buffer->ds_buffer->Play(0, 0, settings.looping ? DSBPLAY_LOOPING : 0);
        sound_buffer->settings = settings;
        sound_buffer->generation++;
        sound_buffer->last_play_time = GetTickCount();
        m_playing_sounds.emplace_back(sound_buffer);
        return {sound_buffer};
    }

    // Use duplicate channel
    SoundDupBuffer* dup_buffer = nullptr;
    for (u32 i = 0; i < MaxDupChannels; ++i) {
        if (m_dup_channels[i].ds_buffer) {
            DWORD dup_status = 0;
            m_dup_channels[i].ds_buffer->GetStatus(&dup_status);
            if (!(dup_status & DSBSTATUS_PLAYING)) {
                m_dup_channels[i].ds_buffer->Release();
                m_dup_channels[i].ds_buffer = nullptr;
            }
        }
        if (!m_dup_channels[i].ds_buffer) {
            dup_buffer = &m_dup_channels[i];
            break;
        }
    }
    if (!dup_buffer) {
        return {};
    }

    m_direct_sound->DuplicateSoundBuffer(sound_buffer->ds_buffer, &dup_buffer->ds_buffer);
    if (!dup_buffer->ds_buffer) {
        return {};
    }
    const auto [volume, pan] = settings.calculate_volume_pan();
    dup_buffer->ds_buffer->SetVolume(volume);
    dup_buffer->ds_buffer->SetPan(pan);
    dup_buffer->ds_buffer->SetCurrentPosition(0);
    dup_buffer->ds_buffer->Play(0, 0, settings.looping ? DSBPLAY_LOOPING : 0);
    dup_buffer->parent_buffer = sound_buffer;
    dup_buffer->settings = settings;
    dup_buffer->generation++;
    dup_buffer->last_play_time = GetTickCount();
    m_playing_sounds.emplace_back(dup_buffer);
    return {dup_buffer};
}

PlayingSound AudioSystem::play_sound(SoundId::Enum sound_id, const PlaySoundSettings& settings) {
    return play_sound(get_sound(sound_id), settings, sound_id);
}

void AudioSystem::start_playback(bool resume) {
    if (m_music_playback.current_music.empty()) {
        return;
    }

    if (resume && m_music_playback.stream) {
        m_music_playback.stream->set_volume(get_playback_volume());
        m_music_playback.stream->play();
        return;
    }

    m_music_playback.stream = MusicStream::open(m_direct_sound, m_music_playback.current_music.c_str());
    if (!m_music_playback.stream) {
        return;
    }

    m_music_playback.stream->set_volume(get_playback_volume());
    m_music_playback.stream->set_pan(0);
    m_music_playback.stream->play();
}

void AudioSystem::stop_playback(bool pause) {
    if (m_music_playback.stream) {
        m_music_playback.stream->stop();
        if (!pause) {
            m_music_playback.stream.reset();
        }
    }
    if (!pause) {
        m_music_playback.current_music.clear();
    }
}

void AudioSystem::shuffle_playlist(Playlist& list) {
    for (u32 i = 0; i < list.tracks.size(); ++i) {
        for (u32 j = i + 1; j < list.tracks.size(); ++j) {
            if (i == 0 && j == list.tracks.size() - 1) {
                // Avoid swapping first and last, so we don't hear the same track twice
                continue;
            }
            if (bw::random_local() % 2 == 0) {
                std::swap(list.tracks[i], list.tracks[j]);
            }
        }
    }
}

void AudioSystem::play_music(std::string_view track) {
    if (is_track_playing(track)) {
        return;
    }
    m_music_playback.next_music = std::string{track};
    m_music_playback.paused_by_trigger = false;
    m_music_playback.target_fade = 0; // Fade out current - update() plays the one-shot next
    ensure_menu_update_timer();
}

void AudioSystem::play_default_music(std::span<const SFilePath> tracks, MusicOptions options) {
    if (!tracks.empty()) {
        u32 first = options.start_index < tracks.size() ? options.start_index : 0;
        if (is_track_playing(tracks[first])) return;
    }

    Playlist& list = m_music_playback.default_list;
    list.tracks.clear();
    for (const auto& track : tracks) {
        if (sfile::file_exists(track.c_str())) {
            list.tracks.push_back(track);
        }
    }
    list.index = options.start_index;
    list.loop = options.loop;
    list.shuffle = options.shuffle;
    list.loop_start = options.loop_start;

    // Default music is the active source only when nothing user-requested is queued
    if (m_music_playback.user_list.tracks.empty() && m_music_playback.next_music.empty()) {
        m_music_playback.target_fade = 0; // Fade out current, then start the new default
    }
    ensure_menu_update_timer();
}

void AudioSystem::skip_music() {
    // Fade out the current track; update() advances to the next (user playlist, then default)
    m_music_playback.target_fade = 0;
}

void AudioSystem::reset_user_music() {
    // Reset user/crig playlist and return control to default (game/menu) music
    stop_playback(false);
    m_music_playback.user_list = {};
    m_music_playback.next_music.clear();
    m_music_playback.paused_by_trigger = false;
    m_music_playback.target_fade = 1;
}

void AudioSystem::pause_music() {
    m_music_playback.paused_by_trigger = true;
    m_music_playback.target_fade = 0;
}

void AudioSystem::resume_music() {
    m_music_playback.paused_by_trigger = false;
    resume_if_unpaused();
}

void AudioSystem::set_music_muted(bool muted) {
    if (m_music_playback.paused_by_volume == muted) return;
    m_music_playback.paused_by_volume = muted;
    if (muted) {
        m_music_playback.target_fade = 0;
        return;
    }
    resume_if_unpaused();
}

void AudioSystem::resume_if_unpaused() {
    if (m_music_playback.is_paused()) return; // Another pause reason still active
    m_music_playback.target_fade = 1;
    ensure_menu_update_timer();
    start_playback(true);
}

void AudioSystem::set_music_fade_in(f32 seconds) {
    m_music_playback.fade_in = seconds;
}

void AudioSystem::set_music_fade_out(f32 seconds) {
    m_music_playback.fade_out = seconds;
}

void AudioSystem::set_music_volume(f32 volume, f32 transition_time) {
    if (transition_time == 0.0f) {
        m_music_playback.volume = volume;
    }
    m_music_playback.target_volume = volume;
    m_music_playback.volume_time = transition_time;
}

void AudioSystem::clear_playlist() {
    m_music_playback.user_list.tracks.clear();
    m_music_playback.user_list.index = 0;
}

bool AudioSystem::add_to_playlist(std::string_view filename) {
    std::string path{filename};
    if (!sfile::file_exists(path.c_str())) {
        return false;
    }
    m_music_playback.user_list.tracks.push_back(std::move(path));
    ensure_menu_update_timer();
    return true;
}

void AudioSystem::set_playlist_shuffle(bool shuffle) {
    m_music_playback.user_list.shuffle = shuffle;
}

void AudioSystem::set_playlist_loop(bool loop) {
    m_music_playback.user_list.loop = loop;
}

s32 AudioSystem::get_sound_volume() const {
    s32 index = 99;
    if (g_use_quiet_sounds) {
        index /= 5;
    }
    return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 AudioSystem::get_sound_volume_at_pos(v2u16 pos) const {
    s32 index = helper::get_sfx_volume_from_screen_distance(pos.x, pos.y);
    if (g_use_quiet_sounds) {
        index /= 5;
    }
    if (index <= 10) {
        return DSBVOLUME_MIN;
    }
    return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 AudioSystem::get_playback_volume() const {
    if (!g_config.play_music_in_background && !display::window_is_focused()) {
        return DSBVOLUME_MIN;
    }
    if (m_music_playback.fade == 0 || m_music_playback.volume == 0) {
        return DSBVOLUME_MIN;
    }

    s32 volume = helper::get_music_volume_centibels();
    volume += (-4000 - volume) * (1 - m_music_playback.fade * m_music_playback.volume);
    return volume;
}

std::string AudioSystem::advance_playlist(Playlist& list) {
    if (list.tracks.empty()) return {};

    if (list.index >= list.tracks.size()) {
        if (!list.loop) return {};

        list.index = list.loop_start;
    }
    if (list.index == 0 && list.shuffle) {
        shuffle_playlist(list);
    }
    return list.tracks[list.index++];
}

std::string AudioSystem::get_next_music() {
    // User playlist takes preference over the default (original/racial) music
    if (!m_music_playback.user_list.tracks.empty()) {
        if (std::string track = advance_playlist(m_music_playback.user_list); !track.empty()) {
            return track;
        }
        clear_playlist(); // Non-looping user playlist finished; drop it so default music resumes
    }
    return advance_playlist(m_music_playback.default_list);
}

void AudioSystem::ensure_menu_update_timer() {
    // In-game the frame loop already drives update(); the timer is only for menus
    if (m_menu_timer_active || g_is_in_game_loop || !g_main_window) {
        return;
    }
    SetTimer(g_main_window, MenuMusicTimerId, MenuMusicTimerIntervalMs, (TIMERPROC)menu_music_timer_proc);
    m_menu_timer_active = true;
}

bool AudioSystem::music_is_playing() const {
    if (!m_music_playback.current_music.empty() || !m_music_playback.next_music.empty() || m_music_playback.is_paused()) {
        return true;
    }
    return !m_music_playback.user_list.tracks.empty() || !m_music_playback.default_list.tracks.empty();
}

bool AudioSystem::is_track_playing(std::string_view track) const {
    return !track.empty() && m_music_playback.current_music == track && m_music_playback.target_fade != 0;
}

void __stdcall AudioSystem::menu_music_timer_proc(HWND window, u32, u32 timer_id, u32) {
    AudioSystem& audio = AudioSystem::instance();
    if (g_is_in_game_loop) {
        KillTimer(window, timer_id);
        audio.m_menu_timer_active = false;
        return;
    }
    audio.update();
    if (!audio.music_is_playing()) {
        KillTimer(window, timer_id);
        audio.m_menu_timer_active = false;
    }
}

VolumePan PlaySoundSettings::calculate_volume_pan() {
    if (!g_config.play_sound_in_background && !display::window_is_focused()) {
        return {DSBVOLUME_MIN, 0};
    }
    if (is_bw_hook) {
        return {bw_volume, bw_pan};
    }

    VolumePan result{99, 0};
    if (get_updated_position()) {
        result.volume = helper::get_sfx_volume_from_screen_distance(position.x, position.y);
        result.pan = helper::get_sfx_pan_from_screen_distance(position.x);

        const auto& tile = bw::get_tile_at_coords(position);
        const auto flags = bw::in_replay() ? ~*g_replay_visions : (u8)(1 << (u32)g_local_nation_id);
        if (tile.visibility_flags & flags) {
            result.volume = DSBVOLUME_MIN;
            return result;
        }
    }
    result.volume = std::max<s32>(result.volume, min_volume);
    result.volume *= g_options_list->sfx;
    if (g_use_quiet_sounds) {
        result.volume /= 5;
    }
    if (result.volume > 0) {
        result.volume = g_big_volume_array[result.volume / 100];
    } else {
        result.volume = DSBVOLUME_MIN;
    }
    return result;
}

void AudioSystem::update_playing_sounds() {
    std::erase_if(m_playing_sounds, [](const PlayingSound& sound) { return !sound.get_buffer(); });

    for (auto& sound : m_playing_sounds) {
        if (auto buffer = sound.get_buffer()) {
            if (buffer->settings.is_bw_hook) {
                continue;
            }

            const auto [volume, pan] = buffer->settings.calculate_volume_pan();
            buffer->ds_buffer->SetVolume(volume);
            buffer->ds_buffer->SetPan(pan);
        }
    }
}

void AudioSystem::stop_and_clear_playing_sounds() {
    for (auto& sound : m_playing_sounds) {
        if (auto buffer = sound.get_buffer()) {
            buffer->ds_buffer->Stop();
            buffer->settings = {};
            buffer->generation = 0;
        }
    }
    m_playing_sounds.clear();
}


// Hooked to stop endless tick sound in scoreboard and other issues
void AudioSystem::stop_all_sounds() {
    // Stop all loaded sounds
    for (u32 i = 0; i < m_loaded_sounds_count; ++i) {
        SoundMainBuffer* sound = &m_loaded_sounds[i];
        if (sound->ds_buffer) {
            sound->ds_buffer->Stop();
        }
    }
    // Stop all duplicate channels
    for (u32 i = 0; i < MaxDupChannels; ++i) {
        SoundDupBuffer* dup_channel = &m_dup_channels[i];
        if (dup_channel->ds_buffer) {
            dup_channel->ds_buffer->Stop();
        }
    }
}

void AudioSystem::reset() {
    stop_and_clear_playing_sounds();

    for (auto& sound : m_dup_channels) {
        sound.free_buffer();
    }

    for (auto& sound : m_loaded_sounds) {
        sound.free_buffer();
    }

    m_loaded_sounds_map.clear();
    m_sound_paths.clear();
    m_playing_sounds.clear();
    m_loaded_sounds_count = 0;

    m_music_playback = {};

    initialize_sound_paths();
}

void PlayingSound::stop() const {
    if (!*this) {
        return;
    }
    m_buffer->ds_buffer->Stop();
}

SoundBuffer* PlayingSound::get_buffer() const {
    if (!*this) {
        return nullptr;
    }
    return m_buffer;
}

PlayingSound::operator bool() const {
    if (!m_buffer || !m_buffer->ds_buffer) {
        return false;
    }
    if (m_buffer->generation != m_generation) {
        return false;
    }

    DWORD status = 0;
    m_buffer->ds_buffer->GetStatus(&status);
    return status & DSBSTATUS_PLAYING;
}