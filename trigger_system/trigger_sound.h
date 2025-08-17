#pragma once
#define NOMINMAX
#include <dsound.h>
#include <vector>
#include "DatExtend/dat_limits.h"
#include "BW/enumerations/SoundId.h"

// Copy paste from Antikythera
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

#define MaxPath 256
#define MaxSound (SoundIdLimit + 5000)
#define MaxSoundHash 512
#define MaxDupChannels 256

struct PlaySoundSettings {
	// Volume and pan are automatically calculated outside of the need of bw hooks
	s32 bw_volume = 1;
	s32 bw_pan = 0;
	bool is_bw_hook = false;

	Point16 position = {};
	Unit* unit = 0;
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
};

struct SoundBuffer {
	LPDIRECTSOUNDBUFFER ds_buffer = nullptr;
	u64 generation = 0;
	PlaySoundSettings settings = {};
	u32 last_play_time = 0;

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
	SoundMainBuffer* hash_next;
	char path[MaxPath];
};

struct SoundDupBuffer : SoundBuffer {
	SoundMainBuffer* parent_buffer;

	bool is_blocking_play(SoundBuffer* primary) override {
		if (primary != parent_buffer) {
			return false;
		}
		return SoundBuffer::is_blocking_play(primary);
	}
};

struct Playlist {
	bool shuffle;
	u32 index;
	u32 size;
	SoundMainBuffer* tracks[MaxSound];
};

class PlayingSound {
public:
	PlayingSound() = default;
	PlayingSound(SoundBuffer* buffer) : m_buffer{buffer}, m_generation{buffer->generation} {}

	void Stop() const;
	SoundBuffer* GetBuffer() const;
	operator bool() const;

private:
	mutable SoundBuffer* m_buffer = 0;
	u64 m_generation = 0;
};

struct MusicPlayback {
	SoundMainBuffer* current_music;
	SoundMainBuffer* next_music;
	f32 fade_in;
	f32 fade_out;
	f32 fade;
	s32 target_fade;
	f32 volume;
	f32 target_volume;
	f32 volume_time;
	bool paused;
};

inline LPDIRECTSOUND g_direct_sound;
inline SoundDupBuffer g_dup_channels[MaxDupChannels];

inline u32 g_loaded_sounds_count;
inline SoundMainBuffer g_loaded_sounds[MaxSound];
inline SoundMainBuffer* g_loaded_sounds_hash_table[MaxSoundHash];

inline Playlist g_current_playlist;
inline MusicPlayback g_music_playback;

inline char sound_paths[SoundIdLimit][MAX_PATH];

inline std::vector<PlayingSound> g_playing_sounds;

void init_sound();
void init_direct_sound();
void update_sound();

SoundMainBuffer* load_sound(const char* filename);
SoundMainBuffer* get_sound(const char* filename); // Note: currently passing `token.string().c_str()`, would be nice to not have to do that
SoundMainBuffer* get_sound(SoundId::Enum sound_id);
void free_sound(SoundMainBuffer* sound);

PlayingSound play_sound_ext(SoundMainBuffer* sound_buffer, PlaySoundSettings settings, SoundId::Enum sound_id = SoundId::None);
PlayingSound play_sound_ext(SoundId::Enum sound_id, const PlaySoundSettings& settings);
void play_music(bool resume);
void stop_music(bool pause);
void shuffle_playlist();

s32 get_sound_volume();
s32 get_sound_volume_at_pos(Point16 pos);
s32 get_playback_volume();
SoundMainBuffer* get_next_music();

struct VolumePan {
	s32 volume;
	s32 pan;
};

VolumePan calculate_volume_pan(PlaySoundSettings& settings);

void update_playing_sounds();
void stop_playing_sounds();
