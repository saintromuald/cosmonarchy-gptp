#pragma once
#include "SCBW/api.h"
#include "trigger_system.h"
#include <dsound.h>

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
#define MaxSound 5000
#define MaxSoundHash 512
#define MaxDupChannels 256

struct Sound {
	Sound* hash_next;
	char path[MaxPath];
	u32 last_play_time;
	bool free_after_play;
	LPDIRECTSOUNDBUFFER ds_buffer;
};

struct Playlist {
	bool shuffle;
	u32 index;
	u32 size;
	Sound* tracks[MaxSound];
};

struct Playback {
	Sound* current_music;
	Sound* next_music;
	f32 fade_in;
	f32 fade_out;
	f32 fade;
	s32 target_fade;
	f32 volume;
	f32 target_volume;
	f32 volume_time;
	bool paused;
};

struct DupChannel {
	s32 unit_id;
	LPDIRECTSOUNDBUFFER ds_buffer;
};

extern LPDIRECTSOUND direct_sound;
extern DupChannel dup_channels[MaxDupChannels];

extern u32 loaded_sounds_count;
extern Sound loaded_sounds[MaxSound];
extern Sound* loaded_sounds_hash_table[MaxSoundHash];

extern Playlist current_playlist;
extern Playback playback;

void init_sound();
void init_direct_sound();
void update_sound();

Sound* load_sound(char* filename);
Sound* get_sound(char* filename);
void free_sound(Sound* sound);

void play_sound(Sound* sound, s32 volume, s32 pan = 0, s32 unit_id = -1);
void play_music(bool resume);
void stop_music();
void shuffle_playlist();

s32 get_sound_volume();
s32 get_sound_volume_at_pos(Point16 pos);
s32 get_playback_volume();
Sound* get_next_music();