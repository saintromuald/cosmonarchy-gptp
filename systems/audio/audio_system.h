#pragma once
#define NOMINMAX
#include <mmreg.h>
#include <dsound.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <span>

#include "../types.h"
#include "../BW/structures/common.h"
#include "../BW/structures/Unit.h"
#include "../BW/enumerations/SoundId.h"
#include "../DatExtend/dat_limits.h"
#include "../BW/bwdata.h"
#include "music_stream.h"
#include "utils/rfl.h"

constexpr auto MaxSound = (SoundIdLimit + 5000);
constexpr auto MaxSoundHash = 512;
constexpr auto MaxDupChannels = 256;
constexpr f32 MusicFadeSeconds = 0.5f; // Menu/music fade duration

struct SoundBuffer;
struct SoundMainBuffer;
struct SoundDupBuffer;

struct VolumePan {
    s32 volume;
    s32 pan;
};

struct PlaySoundSettings {
    // Volume and pan are automatically calculated outside of the need of bw hooks
    s32 bw_volume = 1;
    s32 bw_pan = 0;
    bool is_bw_hook = false;

    v2u16 position = {};
    Unit* unit = nullptr;
    s32 min_volume = 0;
    bool force_globally = false;
    bool looping = false;
    s32 unit_response_id = -1;

    v2u16 get_updated_position() {
        if (force_globally) {
            position = {};
        } else if (unit) {
            position = unit->position;
        }
        return position;
    }

    VolumePan calculate_volume_pan();
};

struct SoundMainBuffer;

struct SoundBuffer {
    IDirectSoundBuffer* ds_buffer = nullptr;
    u64 generation = 0;
    PlaySoundSettings settings = {};
    u32 last_play_time = 0;

    void free_buffer() {
        if (ds_buffer) {
            ds_buffer->Release();
            ds_buffer = nullptr;
        }
    }

    bool is_playing() const {
        if (!ds_buffer) {
            return false;
        }
        DWORD status = 0;
        ds_buffer->GetStatus(&status);
        return status & DSBSTATUS_PLAYING;
    }

    virtual SoundMainBuffer* get_root_buffer() = 0;
    bool is_blocking_play(SoundBuffer* candidate, v2u16 candidate_pos);
};

struct SoundMainBuffer : SoundBuffer {
    SoundMainBuffer* hash_next = nullptr;
    char path[MAX_PATH]{};

    SoundMainBuffer* get_root_buffer() override { return this; }
};

struct SoundDupBuffer : SoundBuffer {
    SoundMainBuffer* parent_buffer = nullptr;

    SoundMainBuffer* get_root_buffer() override { return parent_buffer; }
};

// Handle to a playing sound
class PlayingSound {
public:
    PlayingSound() = default;
    PlayingSound(SoundBuffer* buffer) : m_buffer{buffer}, m_generation{buffer->generation} {}

    void stop() const;
    SoundBuffer* get_buffer() const;
    operator bool() const;

private:
    SoundBuffer* m_buffer = nullptr;
    u64 m_generation = 0;
};

struct Playlist {
    std::vector<std::string> tracks;
    u32 index = 0;
    bool loop = true;
    bool shuffle = false;
    u32 loop_start = 0; // On wrap, jump here instead of 0 (lead-in support)
};

struct MusicPlayback {
    Playlist default_list; // Original/racial music, set by the game via the music hooks
    Playlist user_list;    // Map-authored playlist. Takes preference over default_list

    std::string current_music; 
    std::string next_music;
    std::unique_ptr<MusicStream> stream;
    f32 fade_in = 0;
    f32 fade_out = 0;
    f32 fade = 1;
    s32 target_fade = 1;
    f32 volume = 1;
    f32 target_volume = 1;
    f32 volume_time = 0;
    bool paused_by_trigger = false; // "pause_music" trigger action
    bool paused_by_volume = false;  // Music volume slider at 0

    bool is_paused() const { return paused_by_trigger || paused_by_volume; }
};

struct MusicOptions {
    bool loop = true;
    bool shuffle = false;
    u32 loop_start = 0;
    u32 start_index = 0;
};

class AudioSystem {
public:
    static AudioSystem& instance() {
        static AudioSystem instance;
        return instance;
    }

    // TODO: rule of 0: remove the destructor from manager class and add destructors to the following classes -Veeq7
    // - PlayingSound
    // - SoundBuffer (both SoundMainBuffer and SoundDupBuffer)
    // - DirectSound (as unique_ptr with custom deleter)
    ~AudioSystem();

    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // Main update loop (called every 1/24 sec by game loop)
    void update();

    SoundMainBuffer* load_sound(const std::string& filename);
    SoundMainBuffer* get_sound(const std::string& filename);
    SoundMainBuffer* get_sound(SoundId::Enum sound_id);

    PlayingSound play_sound(SoundMainBuffer* sound_buffer, PlaySoundSettings settings, SoundId::Enum sound_id = SoundId::None);
    PlayingSound play_sound(SoundId::Enum sound_id, const PlaySoundSettings& settings);

    void play_music(std::string_view track); // Map one-shot track ("play_music" trigger)
    void play_default_music(std::span<const SFilePath> tracks, MusicOptions options = {}); // Game/racial music (hooks)
    void skip_music();   // Skip to the next track ("skip_music"/"stop_music" trigger)
    void pause_music();  // "pause_music" trigger action
    void resume_music(); // "resume_music" trigger action
    void set_music_muted(bool muted); // Volume slider hit 0 (or came back up)
    void set_music_fade_in(f32 seconds);
    void set_music_fade_out(f32 seconds);
    void set_music_volume(f32 volume, f32 transition_time);
    void clear_playlist();
    bool add_to_playlist(std::string_view filename);
    void set_playlist_shuffle(bool shuffle);
    void set_playlist_loop(bool loop);

    s32 get_sound_volume() const;
    s32 get_sound_volume_at_pos(v2u16 pos) const;
    s32 get_playback_volume() const;

    void update_playing_sounds();
    void stop_and_clear_playing_sounds();
    void stop_all_sounds(); // Stops all sound buffers without clearing state (for scoreboard, etc.)

    void reset(); // Reset audio state except DirectSound
    void reset_user_music(); // Drop map/user music overrides so default music takes over (game enter/leave)

    MusicPlayback& get_music_playback() { return m_music_playback; }
    const MusicPlayback& get_music_playback() const { return m_music_playback; }

    IDirectSound* get_direct_sound() { return m_direct_sound; }

private:
    AudioSystem();

    void initialize_direct_sound();
    void initialize_sound_paths();

    bool load_ds_buffer(SoundMainBuffer* sound);

    void start_playback(bool resume);
    void stop_playback(bool pause);
    void resume_if_unpaused();
    std::string get_next_music();
    std::string advance_playlist(Playlist& list);
    void shuffle_playlist(Playlist& list);
    void ensure_menu_update_timer();
    bool music_is_playing() const;
    bool is_track_playing(std::string_view track) const; // True when track is the active music and not fading out
    static void __stdcall menu_music_timer_proc(HWND window, u32 message, u32 timer_id, u32 time);

private:
    IDirectSound* m_direct_sound = nullptr;

    // Sound buffers
    std::array<SoundDupBuffer, MaxDupChannels> m_dup_channels;
    std::array<SoundMainBuffer, MaxSound> m_loaded_sounds;
    u32 m_loaded_sounds_count = 0;

    std::unordered_map<std::string, SoundMainBuffer*> m_loaded_sounds_map;
    std::vector<std::array<char, MAX_PATH>> m_sound_paths;
    std::vector<PlayingSound> m_playing_sounds;

    MusicPlayback m_music_playback{};
    bool m_menu_timer_active = false;
};