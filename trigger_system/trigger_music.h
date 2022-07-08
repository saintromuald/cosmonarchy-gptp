#pragma once
#include "SCBW/api.h"
#include "trigger_system.h"

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
#define MaxMusic 256
#define MaxMusicHash 128
struct Music {
	Music* hash_next;
	char path[MaxPath];

	WAVEFORMATEX wave_format;
	size_t size;
	s16* data;
};

struct Playlist {
	bool shuffle;
	u32 index;
	u32 size;
	Music* tracks[MaxMusic];
};

struct Playback {
	Music* current_music;
	Music* next_music;
	f32 fade_in;
	f32 fade_out;
	f32 fade;
	s32 target_fade;
	f32 volume;
	f32 target_volume;
	f32 volume_time;
	bool paused;
};

extern u32 music_tracks_count;
extern Music music_tracks[MaxMusic];
extern Music* music_tracks_hash[MaxMusicHash];

extern Playlist current_playlist;
extern Playback playback;

void init_music();
void init_direct_sound();
void update_music();

Music* load_music(char* filename);
void free_music(Music* music);
Music* get_music(char* filename);

void play_music(Music* music);
void resume_music();
void stop_music();
void shuffle_playlist();

s32 get_volume();
Music* get_next_music();