#include "trigger_music.h"
#include "utils.h"
#include <dsound.h>

static LPDIRECTSOUND direct_sound;
static LPDIRECTSOUNDBUFFER ds_buffer;

Playlist current_playlist;
Playback playback;

u32 music_tracks_count;
Music music_tracks[MaxMusic];
Music* music_tracks_hash[MaxMusicHash];

void init_music() {
	if (!direct_sound) {
		init_direct_sound();
	}

	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}

	for (u32 i = 0; i < MaxMusic; ++i) {
		free_music(&music_tracks[i]);
	}

	music_tracks_count = 0;
	memset(music_tracks, 0, sizeof(music_tracks));
	memset(music_tracks_hash, 0, sizeof(music_tracks_hash));

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

void update_music() {
	if (ds_buffer) {
		DWORD status = 0;
		ds_buffer->GetStatus(&status);
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
				if (ds_buffer) {
					ds_buffer->Stop();
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
			play_music(playback.current_music);
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

	if (ds_buffer) {
		ds_buffer->SetVolume(get_volume());
	}
}

Music* load_music(char* filename) {
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

void free_music(Music* music) {
	if (music->data) {
		free(music->data);
		music->data = 0;
	}
}

Music* get_music(char* filename) {
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

void play_music(Music* music) {
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

void resume_music() {
	if (ds_buffer) {
		ds_buffer->Play(0, 0, 0);
	}
}

void stop_music() {
	if (ds_buffer) {
		ds_buffer->Stop();
		ds_buffer->Release();
		ds_buffer = 0;
	}
	playback.current_music = 0;
}

void shuffle_playlist() {
	for (u32 i = 0; i < current_playlist.size; ++i) {
		Music* music_a = current_playlist.tracks[i];
		for (u32 j = i + 1; j < current_playlist.size; ++j) {
			if (i == 0 && j == current_playlist.size - 1) {
				// Avoid swapping first and last, so we don't hear the same music twice
				continue;
			}
			Music* music_b = current_playlist.tracks[j];
			if (rand() % 2 == 0) {
				Music temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

typedef int __cdecl getMusicVolume_Func();
static getMusicVolume_Func* getMusicVolume = (getMusicVolume_Func*)0x004A5EF0;

s32 get_volume() {
	if (playback.fade == 0 || playback.volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - playback.fade * playback.volume);
	return volume;
}

Music* get_next_music() {
	if (!current_playlist.size) {
		return 0;
	}
	if (current_playlist.index == 0 && current_playlist.shuffle) {
		shuffle_playlist();
	}
	Music* music = current_playlist.tracks[current_playlist.index++];
	if (current_playlist.index >= current_playlist.size) {
		current_playlist.index = 0;
	}
	return music;
}