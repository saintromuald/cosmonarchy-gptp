#pragma once
#include "../types.h"
#include "../BW/structures/common.h"
#include "../BW/structures/Unit.h"
#include "../BW/enumerations/SoundId.h"
#include "../DatExtend/dat_limits.h"
#include "../BW/bwdata.h"

#define NOMINMAX
#include <mmreg.h>
#include <dsound.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <array>
#include <memory>

constexpr auto MaxPath = 256;
constexpr auto MaxSound = (SoundIdLimit + 5000);
constexpr auto MaxSoundHash = 512;
constexpr auto MaxDupChannels = 256;

#define RiffId(a, b, c, d) ((a << 0) | (b << 8) | (c << 16) | (d << 24))

enum RiffFormat {
	RiffFormat_Riff = RiffId('R', 'I', 'F', 'F'),
	RiffFormat_Data = RiffId('d', 'a', 't', 'a'),
	RiffFormat_Fmt = RiffId('f', 'm', 't', ' '),
};

enum WaveFormatEnum {
	WaveFormat_PCM = 0x0001,
	WaveFormat_IEEE_FLOAT = 0x0003,
	WaveFormat_ALAW = 0x0006,
	WaveFormat_MULAW = 0x0007,
	WaveFormat_Extensible = 0xFFFE,
};

#pragma pack(push, 1)
struct WaveHeader {
	u32 riff_id;
	u32 chunk_size;
	u32 wave_id;
};

struct WaveFormat {
	u32 format_id;
	u32 chunk_size;
	u16 format_type;
	u16 channels;
	u32 samples_per_sec;
	u32 average_bytes_per_sec;
	u16 block_align;
	u16 bits_per_sample;
	u16 extension_size;
	u16 valid_bits_per_sample;
	u32 channel_mask;
	u32 sub_format[4];
};

struct WaveData {
	u32 data_id;
	u32 chunk_size;
};
#pragma pack(pop)

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

	Point16 position = {};
	Unit* unit = nullptr;
	s32 min_volume = 0;
	bool force_globally = false;
	bool looping = false;
	s32 unit_response_id = -1;

	Point16 get_updated_position() {
		if (force_globally) {
			position = {};
		} else if (unit) {
			position = unit->position;
		}
		return position;
	}

	VolumePan calculate_volume_pan();
};

struct SoundBuffer {
	LPDIRECTSOUNDBUFFER ds_buffer = nullptr;
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

	virtual bool is_blocking_play(SoundBuffer* primary) {
		if (!last_play_time) {
			return false;
		}
		if (auto pos = settings.get_updated_position()) {
			static constexpr s32 ScreenW = 640;
			static constexpr s32 ScreenH = 400;
			if (pos.x < g_move_to_x || pos.x > g_move_to_x + ScreenW ||
				pos.y < g_move_to_y || pos.y > g_move_to_y + ScreenH) {
				return false;
			}
		}
		return GetTickCount() - last_play_time < 80;
	}
};

struct SoundMainBuffer : SoundBuffer {
	SoundMainBuffer* hash_next = nullptr;
	char path[MaxPath]{};
};

struct SoundDupBuffer : SoundBuffer {
	SoundMainBuffer* parent_buffer = nullptr;

	bool is_blocking_play(SoundBuffer* primary) override {
		if (primary != parent_buffer) {
			return false;
		}
		return SoundBuffer::is_blocking_play(primary);
	}
};

// Handle to a playing sound
class PlayingSound {
public:
	PlayingSound() = default;
	PlayingSound(SoundBuffer* buffer) : m_buffer{buffer}, m_generation{buffer->generation} {}

	void Stop() const;
	SoundBuffer* GetBuffer() const;
	operator bool() const;

private:
	SoundBuffer* m_buffer = nullptr;
	u64 m_generation = 0;
};

struct Playlist {
	bool shuffle = false;
	u32 index = 0;
	u32 size = 0;
	std::array<SoundMainBuffer*, MaxSound> tracks = {};
};

struct MusicPlayback {
	SoundMainBuffer* current_music = nullptr;
	SoundMainBuffer* next_music = nullptr;
	f32 fade_in = 0;
	f32 fade_out = 0;
	f32 fade = 1;
	s32 target_fade = 1;
	f32 volume = 1;
	f32 target_volume = 1;
	f32 volume_time = 0;
	bool paused = false;
};

class AudioSystem {
public:
	static AudioSystem& instance() {
		static AudioSystem instance;
		return instance;
	}

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

	void queue_music(const std::string& filename);
	void stop_music();
	void pause_music();
	void resume_music();
	void set_music_fade_in(f32 seconds);
	void set_music_fade_out(f32 seconds);
	void set_music_volume(f32 volume, f32 transition_time);
	void clear_playlist();
	bool add_to_playlist(const std::string& filename);
	void set_playlist_shuffle(bool shuffle);

	s32 get_sound_volume() const;
	s32 get_sound_volume_at_pos(Point16 pos) const;
	s32 get_playback_volume() const;

	void update_playing_sounds();
	void stop_playing_sounds();
	void stop_all_sounds_partial();  // Stops all sound buffers without clearing state (for scoreboard, etc.)

	void reset();  // Reset audio state except DirectSound

	Playlist& get_playlist() { return m_playlist; }
	const Playlist& get_playlist() const { return m_playlist; }

	MusicPlayback& get_music_playback() { return m_music_playback; }
	const MusicPlayback& get_music_playback() const { return m_music_playback; }

	LPDIRECTSOUND get_direct_sound() { return m_direct_sound; }

private:
	void initialize_direct_sound();
	void initialize_sound_paths();

	bool load_ds_buffer(SoundMainBuffer* sound);


	void play_music(bool resume);
	void stop_music(bool pause);
	SoundMainBuffer* get_next_music();
	void shuffle_playlist();

private:
	AudioSystem();

	LPDIRECTSOUND m_direct_sound = nullptr;

	// Sound buffers
	std::array<SoundDupBuffer, MaxDupChannels> m_dup_channels;
	std::array<SoundMainBuffer, MaxSound> m_loaded_sounds;
	u32 m_loaded_sounds_count = 0;

	std::unordered_map<std::string, SoundMainBuffer*> m_loaded_sounds_map;
	std::vector<std::array<char, MAX_PATH>> m_sound_paths;
	std::vector<PlayingSound> m_playing_sounds;

	Playlist m_playlist{};
	MusicPlayback m_music_playback{};
};