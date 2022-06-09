#include "trigger_system.h"
#include <dsound.h>
#include <limits.h>

#include "token_parser.h"
#include "color_palette.h"

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

static LPDIRECTSOUND direct_sound;
static LPDIRECTSOUNDBUFFER ds_buffer;

static u32 music_tracks_count;
static Music music_tracks[MaxMusic];
static Music* music_tracks_hash[MaxMusicHash];

struct TimedRecolor {
	Recolor shadows;
	Recolor highlights;
	f32 time;
};

#define MaxRecolors 32
struct RecolorList {
	u32 index;
	u32 count;
	TimedRecolor recolors[MaxRecolors];
};

struct TriggerSystem {
	bool playlist_shuffle;
	u32 playlist_index;
	u32 playlist_size;
	Music* playlist_tracks[MaxMusic];

	Music* current_music;
	Music* next_music;
	f32 music_fade_in;
	f32 music_fade_out;
	f32 music_fade;
	s32 music_target_fade;
	f32 music_volume;
	f32 music_target_volume;
	f32 music_volume_time;
	bool music_paused;

	f32 recolor_time;
	TimedRecolor source_recolor;
	TimedRecolor current_recolor;
	TimedRecolor target_recolor;
	RecolorList recolor_list;
};

static bool trigger_system_init;
static bool trigger_system_in_game;
static TriggerSystem trigger_system;

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

typedef int __cdecl getMusicVolume_Func();
static getMusicVolume_Func* getMusicVolume = (getMusicVolume_Func*)0x004A5EF0;

static s32 get_volume() {
	if (trigger_system.music_fade == 0 || trigger_system.music_volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - trigger_system.music_fade * trigger_system.music_volume);
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
	trigger_system.current_music = 0;
}

static void shuffle_playlist() {
	for (u32 i = 0; i < trigger_system.playlist_size; ++i) {
		Music* music_a = trigger_system.playlist_tracks[i];
		for (u32 j = i + 1; j < trigger_system.playlist_size; ++j) {
			if (i == 0 && j == trigger_system.playlist_size - 1) {
				// Avoid swapping first and last, so we don't hear the same music twice
				continue;
			}
			Music* music_b = trigger_system.playlist_tracks[j];
			if (rand() % 2 == 0) {
				Music temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

static Music* get_next_music() {
	if (!trigger_system.playlist_size) {
		return 0;
	}
	if (trigger_system.playlist_index == 0 && trigger_system.playlist_shuffle) {
		shuffle_playlist();
	}
	Music* music = trigger_system.playlist_tracks[trigger_system.playlist_index++];
	if (trigger_system.playlist_index >= trigger_system.playlist_size) {
		trigger_system.playlist_index = 0;
	}
	return music;
}

bool has_custom_name[PlayerLimit];

static b32 __fastcall parse_comment_action(Action* action) {
	if (*currentTriggerPlayerId == *LOCAL_NATION_ID) {
		char* comment = helper::get_chk_string(action->string);

		Parser parser = {};
		parser.at = comment;
		while (parser_get_command(&parser)) {
			if (parser_eq(&parser, "play_music")) {
				Token filename = parser_get_string(&parser, true);
				trigger_system.next_music = get_music(filename.data);
				trigger_system.music_target_fade = 0;
			} else if (parser_eq(&parser, "stop_music")) {
				trigger_system.music_target_fade = 0;
			} else if (parser_eq(&parser, "pause_music")) {
				trigger_system.music_target_fade = 0;
				trigger_system.music_paused = true;
			} else if (parser_eq(&parser, "resume_music")) {
				trigger_system.music_target_fade = 1;
				trigger_system.music_paused = false;
				if (ds_buffer) {
					ds_buffer->Play(0, 0, 0);
				}
			} else if (parser_eq(&parser, "fade_in")) {
				f32 fade_in = parser_get_f32(&parser);
				trigger_system.music_fade_in = fade_in;
			} else if (parser_eq(&parser, "fade_out")) {
				f32 fade_out = parser_get_f32(&parser);
				trigger_system.music_fade_out = fade_out;
			} else if (parser_eq(&parser, "volume")) {
				f32 volume = parser_get_f32(&parser);
				f32 time = parser_get_f32(&parser);
				if (time == 0) {
					trigger_system.music_volume = 0;
				}
				trigger_system.music_target_volume = volume;
				trigger_system.music_volume_time = time;
			} else if (parser_eq(&parser, "playlist_clear")) {
				trigger_system.playlist_size = 0;
			} else if (parser_eq(&parser, "playlist_add")) {
				Token filename = parser_get_string(&parser, true);
				if (trigger_system.playlist_size < MaxMusic) {
					Music* music = get_music(filename.data);
					if (music) {
						trigger_system.playlist_tracks[trigger_system.playlist_size++] = music;
					}
				} else {
					scbw::printf("Error: Playlist already full! (%s)", filename.data);
				}
			} else if (parser_eq(&parser, "playlist_shuffle")) {
				s32 do_shuffle = parser_get_s32(&parser); // 0 - false, 1 - true
				trigger_system.playlist_shuffle = do_shuffle;
			} else if (parser_eq(&parser, "clear_creep")) {
				Box32 location = parser_get_location(&parser);
				scbw::clear_creep(location);
			} else if (parser_eq(&parser, "set_player_name")) {
				s32 player = parser_get_s32(&parser);
				Token name = parser_get_string(&parser, true);
				if (player >= 0 && player < 8) {
					has_custom_name[player] = true;
					if (playerTable[player].type == PlayerType::Human || playerTable[player].type == PlayerType::HumanDefeated) {
						snprintf(playerNameList->player[player].player_name, sizeof(playerNameList->player[player].player_name),
							"%s (%s)", name.data, playerTable[player].name);
					} else {
						snprintf(playerNameList->player[player].player_name, sizeof(playerNameList->player[player].player_name),
							"%s", name.data);
					}

				}
			} else if (parser_eq(&parser, "set_force_name")) {
				s32 player = parser_get_s32(&parser);
				Token name = parser_get_string(&parser, true);
				if (player >= 0 && player < 8) {
					snprintf(playerNameList->player[player].force_name, sizeof(playerNameList->player[player].force_name),
						"%s", name.data);
				}
			} else if (parser_eq(&parser, "recolor")) {
				TimedRecolor recolor  = {};
				recolor.shadows.r     = parser_get_f32(&parser);
				recolor.shadows.g     = parser_get_f32(&parser);
				recolor.shadows.b     = parser_get_f32(&parser);
				recolor.shadows.gamma = parser_get_f32(&parser);
				recolor.highlights    = recolor.shadows;
				recolor.time = parser_get_f32(&parser);

				trigger_system.recolor_time = 0;
				trigger_system.source_recolor = trigger_system.current_recolor;
				trigger_system.target_recolor = recolor;
				if (!recolor.time) {
					trigger_system.current_recolor = recolor;
					set_palette_recolor(recolor.shadows, recolor.highlights);
				}
			} else if (parser_eq(&parser, "recolor_loop_add")) {
				TimedRecolor recolor  = {};
				recolor.shadows.r     = parser_get_f32(&parser);
				recolor.shadows.g     = parser_get_f32(&parser);
				recolor.shadows.b     = parser_get_f32(&parser);
				recolor.shadows.gamma = parser_get_f32(&parser);
				recolor.highlights    = recolor.shadows;
				recolor.time = parser_get_f32(&parser);

				if (trigger_system.recolor_list.count < MaxRecolors) {
					trigger_system.recolor_list.recolors[trigger_system.recolor_list.count++] = recolor;
				}
			} else if (parser_eq(&parser, "recolor_ex")) {
				TimedRecolor recolor  = {};
				recolor.shadows.r     = parser_get_f32(&parser);
				recolor.shadows.g     = parser_get_f32(&parser);
				recolor.shadows.b     = parser_get_f32(&parser);
				recolor.shadows.gamma = parser_get_f32(&parser);
				recolor.highlights.r     = parser_get_f32(&parser);
				recolor.highlights.g     = parser_get_f32(&parser);
				recolor.highlights.b     = parser_get_f32(&parser);
				recolor.highlights.gamma = parser_get_f32(&parser);
				recolor.time = parser_get_f32(&parser);

				trigger_system.recolor_time = 0;
				trigger_system.source_recolor = trigger_system.current_recolor;
				trigger_system.target_recolor = recolor;
				if (!recolor.time) {
					trigger_system.current_recolor = recolor;
					set_palette_recolor(recolor.shadows, recolor.highlights);
				}
			} else if (parser_eq(&parser, "recolor_ex_loop_add")) {
				TimedRecolor recolor  = {};
				recolor.shadows.r     = parser_get_f32(&parser);
				recolor.shadows.g     = parser_get_f32(&parser);
				recolor.shadows.b     = parser_get_f32(&parser);
				recolor.shadows.gamma = parser_get_f32(&parser);
				recolor.highlights.r     = parser_get_f32(&parser);
				recolor.highlights.g     = parser_get_f32(&parser);
				recolor.highlights.b     = parser_get_f32(&parser);
				recolor.highlights.gamma = parser_get_f32(&parser);
				recolor.time = parser_get_f32(&parser);

				if (trigger_system.recolor_list.count < MaxRecolors) {
					trigger_system.recolor_list.recolors[trigger_system.recolor_list.count++] = recolor;
				}
			} else if (parser_eq(&parser, "recolor_loop_clear")) {
				memset(&trigger_system.recolor_list, 0, sizeof(trigger_system.recolor_list));
			}
		}
	}
	return 1;
}

static void init_trigger_system() {
	trigger_system_init = true;
	if (!direct_sound) {
		init_direct_sound();
	}

	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}
	actionTable[47] = parse_comment_action;

	memset(has_custom_name, 0, sizeof(has_custom_name));
	for (u32 i = 0; i < MaxMusic; ++i) {
		free_music(&music_tracks[i]);
	}

	music_tracks_count = 0;
	memset(music_tracks, 0, sizeof(music_tracks));
	memset(music_tracks_hash, 0, sizeof(music_tracks_hash));

	memset(has_custom_name, 0, sizeof(has_custom_name));

	memset(&trigger_system, 0, sizeof(trigger_system));
	trigger_system.music_fade = 1;
	trigger_system.music_target_fade = 1;
	trigger_system.music_volume = 1;
	trigger_system.music_target_volume = 1;

	trigger_system.current_recolor.shadows.r     = 1;
	trigger_system.current_recolor.shadows.g     = 1;
	trigger_system.current_recolor.shadows.b     = 1;
	trigger_system.current_recolor.shadows.gamma = 1;
	trigger_system.current_recolor.highlights    = trigger_system.current_recolor.shadows;

	trigger_system.target_recolor.shadows        = trigger_system.current_recolor.shadows;
	trigger_system.target_recolor.highlights     = trigger_system.current_recolor.shadows;
}

void handle_music() {
	if (ds_buffer) {
		DWORD status = 0;
		ds_buffer->GetStatus(&status);
		if (status != DSBSTATUS_PLAYING) {
			trigger_system.current_music = 0;
		}
	} else {
		trigger_system.current_music = 0;
	}

	if (trigger_system.music_target_fade == 0) {
		if (trigger_system.music_fade_out) {
			trigger_system.music_fade -= (1.0 / trigger_system.music_fade_out) / 24.0;
		} else {
			trigger_system.music_fade = 0;
		}
		if (trigger_system.music_fade <= 0) {
			trigger_system.music_fade = 0;

			if (trigger_system.music_paused) {
				if (ds_buffer) {
					ds_buffer->Stop();
				}
			} else {
				stop_music();
			}
		}
	}

	if (!trigger_system.current_music && !trigger_system.music_paused) {
		if (!trigger_system.next_music) {
			trigger_system.next_music = get_next_music();
		}
		trigger_system.current_music = trigger_system.next_music;
		trigger_system.next_music = get_next_music();
		if (trigger_system.current_music) {
			trigger_system.music_target_fade = 1;
			play_music(trigger_system.current_music);
		}
	}

	if (trigger_system.current_music || trigger_system.music_paused) {
		if (*musicPlaying) {
			helper::stopMusic();
		}
	} else {
		if (!*musicPlaying) {
			helper::playNextMusic();
		}
	}

	if (trigger_system.music_target_fade == 1) {
		if (trigger_system.music_fade_in) {
			trigger_system.music_fade += (1.0 / trigger_system.music_fade_in) / 24.0;
		} else {
			trigger_system.music_fade = 1;
		}
		if (trigger_system.music_fade >= 1) {
			trigger_system.music_fade = 1;
		}
	}

	if (trigger_system.music_volume_time) {
		f32 music_volume_delta = (1.0 / trigger_system.music_volume_time) / 24.0;
		if (trigger_system.music_volume < trigger_system.music_target_volume) {
			trigger_system.music_volume += music_volume_delta;
			if (trigger_system.music_volume >= trigger_system.music_target_volume) {
				trigger_system.music_volume = trigger_system.music_target_volume;
			}
		} else {
			trigger_system.music_volume -= music_volume_delta;
			if (trigger_system.music_volume <= trigger_system.music_target_volume) {
				trigger_system.music_volume = trigger_system.music_target_volume;
			}
		}
	} else {
		trigger_system.music_volume = trigger_system.music_target_volume;
	}

	if (ds_buffer) {
		ds_buffer->SetVolume(get_volume());
	}
}

void handle_recolor() {
	if (*elapsed_time_frames == 0) {
		set_palette_recolor({1, 1, 1, 1}, {1, 1, 1, 1});
	}
	if (trigger_system.recolor_time < trigger_system.target_recolor.time) {
		trigger_system.recolor_time += 1.0 / 24; // Tick Time for Brood War

		f32 t = trigger_system.recolor_time / trigger_system.target_recolor.time;
		if (t > 1) {
			t = 1;
		}

		trigger_system.current_recolor.shadows    = lerp_recolor(t, trigger_system.current_recolor.shadows,    trigger_system.target_recolor.shadows);
		trigger_system.current_recolor.highlights = lerp_recolor(t, trigger_system.current_recolor.highlights, trigger_system.target_recolor.highlights);

		set_palette_recolor(trigger_system.current_recolor.shadows, trigger_system.current_recolor.highlights);
	} else if (trigger_system.recolor_list.count) {
		trigger_system.recolor_list.index %= trigger_system.recolor_list.count; // In case the list got smaller

		trigger_system.recolor_time = 0;
		trigger_system.source_recolor = trigger_system.current_recolor;
		trigger_system.target_recolor = trigger_system.recolor_list.recolors[trigger_system.recolor_list.index];

		trigger_system.recolor_list.index += 1;
		trigger_system.recolor_list.index %= trigger_system.recolor_list.count;
	}
}

void tick_trigger_system(bool in_game) {
	if (!trigger_system_init || (*is_in_game_loop && *elapsed_time_frames == 0)) {
		init_trigger_system();
	}

	if (in_game) {
		trigger_system_in_game = true;
	} else if (trigger_system_in_game) {
		// Leaving the game should reset music
		trigger_system_in_game = false;
		init_trigger_system();
		set_palette_recolor({1, 1, 1, 1}, {1, 1, 1, 1});
	}
	
	handle_music();
	if (in_game) {
		handle_recolor();
	}
}