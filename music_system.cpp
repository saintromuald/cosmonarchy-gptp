#include <dsound.h>
#include <limits.h>
#include "music_system.h"
#include "SCBW/api.h"

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

struct LoadedFile {
	size_t size;
	char* data;
};

static LoadedFile load_entire_sfile(char* filename) {
	HANDLE file = 0;
	helper::SFileOpenFile(filename, &file);
	if (!file) {
		scbw::printf("Error: File %s missing!", filename);
		return {};
	}
	
	DWORD high = 0;
	DWORD size = helper::SFileGetFileSize(file, &high);
	if (high) {
		scbw::printf("Error: File %s too big!", filename);
		return {};
	}

	LoadedFile loaded_file = {};
	loaded_file.size = size;
	if (loaded_file.size) {
		loaded_file.data = (char*) calloc(loaded_file.size + 1, 1);
		helper::SFileReadFile(file, loaded_file.data, loaded_file.size, 0, 0);
		loaded_file.data[loaded_file.size] = 0;
	}
	helper::SFileCloseFile(file);

	return loaded_file;
}

static void free_loaded_file(LoadedFile* file) {
	if (file->data) {
		free(file->data);
	}
	file->size = 0;
	file->data = 0;
}

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

static bool music_init;
static bool music_in_game;
static LPDIRECTSOUND direct_sound;
static LPDIRECTSOUNDBUFFER ds_buffer;

static u32 music_tracks_count;
static Music music_tracks[MaxMusic];
static Music* music_tracks_hash[MaxMusicHash];

static bool playlist_shuffle;
static u32 playlist_index;
static u32 playlist_size;
static Music* playlist_tracks[MaxMusic];

static Music* current_music;
static Music* next_music;
static f32 music_fade_in;
static f32 music_fade_out;
static f32 music_fade;
static s32 music_target_fade;
static f32 music_volume;
static f32 music_target_volume;
static f32 music_volume_time;
static bool music_paused;

#define MaxToken 256
struct Token {
	u32 size;
	char data[MaxToken];
};

struct Parser {
	char* at;
	Token token;
};

static u32 hash_string(char* string) {
	u32 hash = 5381;
	for (char* c = string; *c; ++c) {
		hash = ((hash << 5) + hash) + *c;
	}
	return hash;
}

static Music* load_music(char* filename) {
	if (music_tracks_count < MaxMusic) {
		LoadedFile file = load_entire_sfile(filename);
		if (!file.data) {
			scbw::printf("Error: File %s missing!", filename);
			return 0;
		}

		Music* music = &music_tracks[music_tracks_count++];
		strcpy_s(music->path, filename);

		u8* at = (u8*)file.data;
		void* end = file.data + file.size;
		while (at < end) {
			u32* type = (u32*)at;
			switch (*type) {
				case RiffFormat_Riff: {
					WaveHeader* header = (WaveHeader*) at;
					at += sizeof(*header);
				} break;
				case RiffFormat_Data: {
					WaveData* data = (WaveData*) at;
					at += data->chunk_size + 8;

					music->size = data->chunk_size;
					music->data = (s16*)calloc(1, music->size);
					memcpy(music->data, data + 1, music->size);
				} break;
				case RiffFormat_Fmt: {
					WaveFormat* format = (WaveFormat*) at;
					at += format->chunk_size + 8;
					music->wave_format.wFormatTag = format->format_type;
					music->wave_format.nChannels = format->channels;
					music->wave_format.nSamplesPerSec = format->samples_per_sec;
					music->wave_format.nAvgBytesPerSec = format->average_bytes_per_sec;
					music->wave_format.nBlockAlign = format->block_align;
					music->wave_format.wBitsPerSample = format->bits_per_sample;
				} break;
				default: {
					at = (u8*)end;
					// @Todo: log an error
				} break;
			}
		}
		free_loaded_file(&file);

		u32 hash_index = hash_string(filename) % MaxMusicHash;
		music->hash_next = music_tracks_hash[hash_index];
		music_tracks_hash[hash_index] = music;
		return music;
	}
	return 0;
}

static void free_music(Music* music) {
	if (music->data) {
		free(music->data);
		music->data = 0;
	}
}

static Music* get_music(char* filename) {
	u32 hash_index = hash_string(filename) % MaxMusicHash;
	Music* music = music_tracks_hash[hash_index];
	while (music && strcmp(music->path, filename) != 0) {
		music = music->hash_next;
	}
	if (!music) {
		music = load_music(filename);
	}
	return music;
}

static void init_direct_sound() {
	// Init DirectSound
	if (SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0))) {
		WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = 48000;
		wave_format.wBitsPerSample = 16;
		wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;

		if (SUCCEEDED(direct_sound->SetCooperativeLevel(*(HWND*)mainWindow, DSSCL_PRIORITY))) {
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(DSBUFFERDESC);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			// @Todo: what is a primary buffer needed for? seems like sounds play without too 
			LPDIRECTSOUNDBUFFER primary_buffer = 0;
			if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0))) {
				primary_buffer->SetFormat(&wave_format);
			}
		}
	}
}

static s32 get_volume() {
	if (music_fade == 0 || music_volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = *musicVolume;
	volume += (-4000 - volume) * (1 - music_fade * music_volume);
	return volume;
}

static void play_music(Music* music) {
	if (!direct_sound) {
		return;
	}

	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}

	DSBUFFERDESC buffer_description = {};
	buffer_description.dwSize = sizeof(DSBUFFERDESC);
	buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	buffer_description.dwBufferBytes = music->size;
	buffer_description.lpwfxFormat = &music->wave_format;

	if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &ds_buffer, 0))) {
		return;
	}

	ds_buffer->SetVolume(get_volume());

	u32 play_cursor = 0;
	u32 write_cursor = 0;
	if (SUCCEEDED(ds_buffer->GetCurrentPosition((LPDWORD)&play_cursor, (LPDWORD)&write_cursor))) {
		u8* ptr1 = 0;
		u32 bytes1 = 0;
		u8* ptr2 = 0;
		u32 bytes2 = 0;

		if (SUCCEEDED(ds_buffer->Lock(write_cursor, music->size, (LPVOID*)&ptr1, (LPDWORD)&bytes1, (LPVOID*)&ptr2, (LPDWORD)&bytes2, 0))) {
			if (ptr1) {
				s16* out = (s16*) ptr1;
				for (u32 i = 0; i < bytes1 / sizeof(s16); ++i) {
					*out++ = music->data[i];
				}
			}
			if (ptr2) {
				s16* out = (s16*) ptr2;
				for (u32 i = 0; i < bytes2 / sizeof(s16); ++i) {
					*out++ = music->data[i];
				}
			}

			ds_buffer->Unlock(ptr1, bytes1, ptr2, bytes2);
		}
	}

	ds_buffer->Play(0, 0, 0);
}

static void stop_music() {
	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}
	current_music = 0;
}

static bool parser_get_word(Parser* parser) {
	// Skip Whitespace
	while (*parser->at && isspace(*parser->at)) {
		++parser->at;
	}

	// Write Token
	memset(&parser->token, 0, sizeof(parser->token));
	while (*parser->at && !isspace(*parser->at)) {
		if (parser->token.size < MaxToken - 1) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	return parser->token.size != 0;
}

static Token parser_get_token(Parser* parser) {
	parser_get_word(parser);
	return parser->token;
}

static s32 parser_get_s32(Parser* parser, u32 radix = 10) {
	parser_get_word(parser);
	return strtol(parser->token.data, 0, radix);
}

static f32 parser_get_f32(Parser* parser) {
	parser_get_word(parser);
	return strtof(parser->token.data, 0);
}

static bool parser_eq(Parser* parser, char* string) {
	if (!string) return false;
	return strcmp(parser->token.data, string) == 0;
}

static b32 __fastcall parse_comment_action(ACTION* action) {
	if (*currentTriggerPlayerId == *LOCAL_NATION_ID) {
		char* comment = helper::get_chk_string(action->string);

		Parser parser = {};
		parser.at = comment;
		while (parser_get_word(&parser)) {
			if (parser_eq(&parser, "play_music")) {
				Token filename = parser_get_token(&parser);
				next_music = get_music(filename.data);
				music_target_fade = 0;
			} else if (parser_eq(&parser, "stop_music")) {
				music_target_fade = 0;
			} else if (parser_eq(&parser, "pause_music")) {
				music_target_fade = 0;
				music_paused = true;
			} else if (parser_eq(&parser, "resume_music")) {
				music_target_fade = 1;
				music_paused = false;
				if (ds_buffer) {
					ds_buffer->Play(0, 0, 0);
				}
			} else if (parser_eq(&parser, "fade_in")) {
				f32 fade_in = parser_get_f32(&parser);
				music_fade_in = fade_in;
			} else if (parser_eq(&parser, "fade_out")) {
				f32 fade_out = parser_get_f32(&parser);
				music_fade_out = fade_out;
			} else if (parser_eq(&parser, "volume")) {
				f32 volume = parser_get_f32(&parser);
				f32 time = parser_get_f32(&parser);
				if (time == 0) {
					music_volume = 0;
				}
				music_target_volume = volume;
				music_volume_time = time;
			} else if (parser_eq(&parser, "playlist_clear")) {
				playlist_size = 0;
			} else if (parser_eq(&parser, "playlist_add")) {
				Token filename = parser_get_token(&parser);
				if (playlist_size < MaxMusic) {
					playlist_tracks[playlist_size++] = get_music(filename.data);
				} else {
					scbw::printf("Error: Playlist already fully! (%s)", filename.data);
				}
			} else if (parser_eq(&parser, "playlist_shuffle")) {
				s32 do_shuffle = parser_get_s32(&parser); // 0 - false, 1 - true
				playlist_shuffle = do_shuffle;
			}
		}
	}
	return 1;
}

static void shuffle_playlist() {
	for (u32 i = 0; i < playlist_size; ++i) {
		Music* music_a = playlist_tracks[i];
		for (u32 j = i + 1; j < playlist_size; ++j) {
			Music* music_b = playlist_tracks[j];
			if (rand() % 2 == 0) {
				Music temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

static Music* get_next_music() {
	if (!playlist_size) {
		return 0;
	}
	if (playlist_index == 0) {
		shuffle_playlist();
	}
	Music* music = playlist_tracks[playlist_index++];
	if (playlist_index >= playlist_size) {
		playlist_index = 0;
	}
	return music;
}

static void init_music_system() {
	music_init = true;
	if (!direct_sound) {
		init_direct_sound();
	}

	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}
	actionTable[47] = parse_comment_action;

	for (u32 i = 0; i < MaxMusic; ++i) {
		free_music(&music_tracks[i]);
	}
	music_tracks_count = 0;
	memset(music_tracks, 0, sizeof(music_tracks));
	memset(music_tracks_hash, 0, sizeof(music_tracks_hash));

	playlist_shuffle = false;
	playlist_index = 0;
	playlist_size = 0;
	memset(playlist_tracks, 0, sizeof(playlist_tracks));

	current_music = 0;
	next_music = 0;
	music_fade_in = 0;
	music_fade_out = 0;
	music_fade = 1;
	music_target_fade = 1;
	music_volume = 1;
	music_target_volume = 1;
	music_volume_time = 0;
	music_paused = false;
}

void tick_music_system(bool in_game) {
	if (!music_init || (*IS_IN_GAME_LOOP && *elapsedTimeFrames == 0)) {
		init_music_system();
	}

	if (in_game) {
		music_in_game = true;
	} else if (music_in_game) {
		// Leaving the game should reset music
		music_in_game = false;
		init_music_system();
	}
	
	if (ds_buffer) {
		DWORD status = 0;
		ds_buffer->GetStatus(&status);
		if (status != DSBSTATUS_PLAYING) {
			current_music = 0;
		}
	} else {
		current_music = 0;
	}

	if (music_target_fade == 0) {
		if (music_fade_out) {
			music_fade -= (1.0 / music_fade_out) / 24.0;
		} else {
			music_fade = 0;
		}
		if (music_fade <= 0) {
			music_fade = 0;

			if (music_paused) {
				if (ds_buffer) {
					ds_buffer->Stop();
				}
			} else {
				stop_music();
			}
		}
	}

	if (!current_music && !music_paused) {
		if (!next_music) {
			next_music = get_next_music();
		}
		current_music = next_music;
		next_music = get_next_music();
		if (current_music) {
			music_target_fade = 1;
			play_music(current_music);
		}
	}

	if (current_music || music_paused) {
		if (*musicPlaying) {
			helper::stopMusic();
		}
	} else {
		if (!*musicPlaying) {
			helper::playNextMusic();
		}
	}

	if (music_target_fade == 1) {
		if (music_fade_in) {
			music_fade += (1.0 / music_fade_in) / 24.0;
		} else {
			music_fade = 1;
		}
		if (music_fade >= 1) {
			music_fade = 1;
		}
	}

	if (music_volume_time) {
		f32 music_volume_delta = (1.0 / music_volume_time) / 24.0;
		if (music_volume < music_target_volume) {
			music_volume += music_volume_delta;
			if (music_volume >= music_target_volume) {
				music_volume = music_target_volume;
			}
		} else {
			music_volume -= music_volume_delta;
			if (music_volume <= music_target_volume) {
				music_volume = music_target_volume;
			}
		}
	} else {
		music_volume = music_target_volume;
	}

	if (ds_buffer) {
		ds_buffer->SetVolume(get_volume());
	}
}