#include "trigger_sound.h"
#include "utils.h"

static LPDIRECTSOUND direct_sound;
static LPDIRECTSOUNDBUFFER duplicated_buffers[MaxDupChannels];

Playlist current_playlist;
Playback playback;

u32 loaded_sounds_count;
Sound loaded_sounds[MaxSound];
Sound* loaded_sounds_hash_table[MaxSoundHash];

void init_sound() {
	if (!direct_sound) {
		init_direct_sound();
	}

	for (u32 i = 0; i < MaxDupChannels; ++i) {
		if (duplicated_buffers[i]) {
			duplicated_buffers[i]->Stop();
			duplicated_buffers[i]->Release();
			duplicated_buffers[i] = 0;
		}
	}

	for (u32 i = 0; i < MaxSound; ++i) {
		free_sound(&loaded_sounds[i]);
	}

	loaded_sounds_count = 0;
	memset(loaded_sounds, 0, sizeof(loaded_sounds));
	memset(loaded_sounds_hash_table, 0, sizeof(loaded_sounds_hash_table));

	memset(&playback, 0, sizeof(playback));
	playback.fade = 1;
	playback.target_fade = 1;
	playback.volume = 1;
	playback.target_volume = 1;
}

void init_direct_sound() {
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

		if (SUCCEEDED(direct_sound->SetCooperativeLevel(*(HWND*)g_main_window, DSSCL_PRIORITY))) {
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

void update_sound() {
	if (playback.current_music && playback.current_music->ds_buffer) {
		DWORD status = 0;
		playback.current_music->ds_buffer->GetStatus(&status);
		if (status != DSBSTATUS_PLAYING) {
			playback.current_music = 0;
		}
	} else {
		playback.current_music = 0;
	}

	if (playback.target_fade == 0) {
		if (playback.fade_out) {
			playback.fade -= (1.0 / playback.fade_out) / 24.0;
		} else {
			playback.fade = 0;
		}
		if (playback.fade <= 0) {
			playback.fade = 0;

			if (playback.paused) {
				if (playback.current_music && playback.current_music->ds_buffer) {
					playback.current_music->ds_buffer->Stop();
				}
			} else {
				stop_music();
			}
		}
	}

	if (!playback.current_music && !playback.paused) {
		if (!playback.next_music) {
			playback.next_music = get_next_music();
		}
		playback.current_music = playback.next_music;
		playback.next_music = get_next_music();
		if (playback.current_music) {
			playback.target_fade = 1;
			resume_music();
		}
	}

	if (playback.current_music || playback.paused) {
		if (*g_music_playing) {
			helper::stopMusic();
		}
	} else {
		if (!*g_music_playing) {
			helper::playNextMusic();
		}
	}

	if (playback.target_fade == 1) {
		if (playback.fade_in) {
			playback.fade += (1.0 / playback.fade_in) / 24.0;
		} else {
			playback.fade = 1;
		}
		if (playback.fade >= 1) {
			playback.fade = 1;
		}
	}

	if (playback.volume_time) {
		f32 music_volume_delta = (1.0 / playback.volume_time) / 24.0;
		if (playback.volume < playback.target_volume) {
			playback.volume += music_volume_delta;
			if (playback.volume >= playback.target_volume) {
				playback.volume = playback.target_volume;
			}
		} else {
			playback.volume -= music_volume_delta;
			if (playback.volume <= playback.target_volume) {
				playback.volume = playback.target_volume;
			}
		}
	} else {
		playback.volume = playback.target_volume;
	}

	if (playback.current_music && playback.current_music->ds_buffer) {
		playback.current_music->ds_buffer->SetVolume(get_playback_volume());
	}
}

static LPDIRECTSOUNDBUFFER load_ds_buffer(Sound* sound, SoundLoadData* sound_load_data) {
	if (!direct_sound) {
		return 0;
	}

	DSBUFFERDESC buffer_description = {};
	buffer_description.dwSize = sizeof(DSBUFFERDESC);
	buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	buffer_description.dwBufferBytes = sound_load_data->size;
	buffer_description.lpwfxFormat = &sound_load_data->wave_format;

	if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &sound->ds_buffer, 0))) {
		return 0;
	}

	u32 play_cursor = 0;
	u32 write_cursor = 0;
	if (SUCCEEDED(sound->ds_buffer->GetCurrentPosition((LPDWORD)&play_cursor, (LPDWORD)&write_cursor))) {
		u8* ptr1 = 0;
		u32 bytes1 = 0;
		u8* ptr2 = 0;
		u32 bytes2 = 0;

		if (SUCCEEDED(sound->ds_buffer->Lock(write_cursor, sound_load_data->size, (LPVOID*)&ptr1, (LPDWORD)&bytes1, (LPVOID*)&ptr2, (LPDWORD)&bytes2, 0))) {
			if (ptr1) {
				s16* out = (s16*) ptr1;
				for (u32 i = 0; i < bytes1 / sizeof(s16); ++i) {
					*out++ = sound_load_data->data[i];
				}
			}
			if (ptr2) {
				s16* out = (s16*) ptr2;
				for (u32 i = 0; i < bytes2 / sizeof(s16); ++i) {
					*out++ = sound_load_data->data[i];
				}
			}

			sound->ds_buffer->Unlock(ptr1, bytes1, ptr2, bytes2);
		}
	}
}

Sound* load_sound(char* filename) {
	if (loaded_sounds_count < MaxSound) {
		LoadedFile file = load_entire_sfile(filename);
		if (!file.data) {
			scbw::printf("Error: File %s missing!", filename);
			return 0;
		}

		SoundLoadData sound_load_data = {};

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

					sound_load_data.size = data->chunk_size;
					sound_load_data.data = (s16*)calloc(1, sound_load_data.size);
					memcpy(sound_load_data.data, data + 1, sound_load_data.size);
				} break;
				case RiffFormat_Fmt: {
					WaveFormat* format = (WaveFormat*) at;
					at += format->chunk_size + 8;
					sound_load_data.wave_format.wFormatTag = format->format_type;
					sound_load_data.wave_format.nChannels = format->channels;
					sound_load_data.wave_format.nSamplesPerSec = format->samples_per_sec;
					sound_load_data.wave_format.nAvgBytesPerSec = format->average_bytes_per_sec;
					sound_load_data.wave_format.nBlockAlign = format->block_align;
					sound_load_data.wave_format.wBitsPerSample = format->bits_per_sample;
				} break;
				default: {
					at = (u8*)end;
					// @Todo: log an error
				} break;
			}
		}
		free_loaded_file(&file);

		Sound* sound = &loaded_sounds[loaded_sounds_count++];
		strcpy_s(sound->path, filename);

		load_ds_buffer(sound, &sound_load_data);
		if (sound_load_data.data) {
			free(sound_load_data.data);
		}

		u32 hash_index = hash_string(filename) % MaxSoundHash;
		sound->hash_next = loaded_sounds_hash_table[hash_index];
		loaded_sounds_hash_table[hash_index] = sound;
		return sound;
	}
	return 0;
}

void free_sound(Sound* sound) {
	if (sound->ds_buffer) {
		sound->ds_buffer->Stop();
		sound->ds_buffer->Release();
		sound->ds_buffer = 0;
	}
}

Sound* get_sound(char* filename) {
	u32 hash_index = hash_string(filename) % MaxSoundHash;
	Sound* sound = loaded_sounds_hash_table[hash_index];
	while (sound && strcmp(sound->path, filename) != 0) {
		sound = sound->hash_next;
	}
	if (!sound) {
		sound = load_sound(filename);
	}
	return sound;
}

void play_sound(Sound* sound, s32 volume) {
	if (!sound) {
		return;
	}
	if (sound->last_play_time && *g_elapsed_time_frames - sound->last_play_time < 3) {
		return;
	}

	DWORD status = 0;
	if (!SUCCEEDED(sound->ds_buffer->GetStatus(&status)) || status == DSBSTATUS_PLAYING) {
		// Use dup channel

		LPDIRECTSOUNDBUFFER* buffer = 0;
		for (u32 i = 0; i < MaxDupChannels; ++i) {
			if (duplicated_buffers[i]) {
				DWORD current_status = 0;
				if (!SUCCEEDED(duplicated_buffers[i]->GetStatus(&status)) || current_status != DSBSTATUS_PLAYING) {
					duplicated_buffers[i]->Stop();
					duplicated_buffers[i]->Release();
					duplicated_buffers[i] = 0;
				}
			}
			if (!duplicated_buffers[i]) {
				buffer = &duplicated_buffers[i];
			}
		}

		if (buffer) {
			direct_sound->DuplicateSoundBuffer(sound->ds_buffer, buffer);
			if (*buffer) {
				(*buffer)->SetVolume(volume);
				(*buffer)->Play(0, 0, 0);
				sound->last_play_time = *g_elapsed_time_frames;
			}
		}
	} else {
		sound->ds_buffer->SetVolume(volume);
		sound->ds_buffer->Play(0, 0, 0);
		sound->last_play_time = *g_elapsed_time_frames;
	}
}

void resume_music() {
	if (playback.current_music && playback.current_music->ds_buffer) {
		playback.current_music->ds_buffer->Play(0, 0, 0);
	}
}

void stop_music() {
	if (playback.current_music->ds_buffer) {
		playback.current_music->ds_buffer->Stop();
	}
	playback.current_music = 0;
}

void shuffle_playlist() {
	for (u32 i = 0; i < current_playlist.size; ++i) {
		Sound* music_a = current_playlist.tracks[i];
		for (u32 j = i + 1; j < current_playlist.size; ++j) {
			if (i == 0 && j == current_playlist.size - 1) {
				// Avoid swapping first and last, so we don't hear the same sound twice
				continue;
			}
			Sound* music_b = current_playlist.tracks[j];
			if (rand() % 2 == 0) {
				Sound temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

typedef int __cdecl getMusicVolume_Func();
static getMusicVolume_Func* getMusicVolume = (getMusicVolume_Func*)0x004A5EF0;

s32 get_sound_volume() {
	return g_big_volume_array[99 * options_list->sfx / 100];
}

s32 get_sound_volume_at_pos(Point16 pos) {
	u32 index = helper::get_sfx_volume_from_screen_distance(pos.x, pos.y);
	if (*use_quiet_sounds) {
		index /= 5;
	}
	if (index <= 10) {
		return DSBVOLUME_MIN;
	}
	return g_big_volume_array[index * options_list->sfx / 100];
}

s32 get_playback_volume() {
	if (playback.fade == 0 || playback.volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - playback.fade * playback.volume);
	return volume;
}

Sound* get_next_music() {
	if (!current_playlist.size) {
		return 0;
	}
	if (current_playlist.index == 0 && current_playlist.shuffle) {
		shuffle_playlist();
	}
	Sound* music = current_playlist.tracks[current_playlist.index++];
	if (current_playlist.index >= current_playlist.size) {
		current_playlist.index = 0;
	}
	return music;
}